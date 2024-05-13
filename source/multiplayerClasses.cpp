#include "multiplayerClasses.h"
#include "scene.hpp"
#include <stdexcept>

template<typename T>
void clearQueue(std::queue<T>& q) {
    std::queue<T> empty;
    std::swap(q, empty);
}

u8 getEnemyIndex(u8 id) {
    if(id > multiplayerLink->universal->currentPlayerId())
        id -= 1;
    return id;
}

RemotePlayerId::RemotePlayerId(u8 playerId) {
    id = playerId;
    index = getEnemyIndex(playerId);
}

void MultiplayerLink::activate() {
    active = true;
    universal->activate();
}

bool MultiplayerLink::sync() {
    // Do universal sync
    UpdateStateMachine();

    // Beat state to other players
    if (universal->isConnected()) {

        auto limiter = LINK_WIRELESS_QUEUE_SIZE * 2;
        auto exitEarly = false;
#ifdef GBA
        if (sendQueue.size() > limiter * 3) {
            if (stateMachine.IsInState<MultiplayerStates::PlayersSeeded>()
                    || stateMachine.IsInState<MultiplayerStates::Playing>()
                    || stateMachine.IsInState<MultiplayerStates::Lost>())
                setUpPresentException("Send queue exceeded safe limit.");
            else
                clearQueue(sendQueue);
        }
        if (seedQueue.size() > limiter * 3) {
            if (stateMachine.IsInState<MultiplayerStates::PlayersSeeded>()
                || stateMachine.IsInState<MultiplayerStates::Playing>()
                || stateMachine.IsInState<MultiplayerStates::Lost>())
            setUpPresentException("Seed queue exceeded safe limit.");
            else
                clearQueue(seedQueue);
        }
#endif
        while (closed() && !exitEarly && !sendQueue.empty() && --limiter) {
            auto front = sendQueue.front();
            bool sent = universal->send(front);
            if (sent)
                sendQueue.pop();
            else {
#ifdef GBA
                std::string error;
                switch (universal->linkWireless->getLastError(true)) {
                    case LinkWireless::WRONG_STATE:
                        error = "Wrong state       ";
                        break;
                    case LinkWireless::GAME_NAME_TOO_LONG:
                        error = "Game name too long";
                        break;
                    case LinkWireless::USER_NAME_TOO_LONG:
                        error = "User name too long";
                        break;
                    case LinkWireless::BUFFER_IS_FULL:
                        // Hopefully it'll catch up, keep trying
                        exitEarly = true;
                        break;
                    case LinkWireless::COMMAND_FAILED:
                        error = "Command failed    ";
                        break;
                    case LinkWireless::CONNECTION_FAILED:
                        error = "Connection failed ";
                        break;
                    case LinkWireless::SEND_DATA_FAILED:
                        error = "Send data failed  ";
                        break;
                    case LinkWireless::RECEIVE_DATA_FAILED:
                        error = "Recv data failed  ";
                        break;
                    case LinkWireless::ACKNOWLEDGE_FAILED:
                        error = "ACK failed        ";
                        break;
                    case LinkWireless::NONE:
                        break;
                    case LinkWireless::TIMEOUT:
                    case LinkWireless::REMOTE_TIMEOUT:
                        exitEarly = true;
                        break;
                }
                if (!error.empty()) {
                    setUpPresentException(error);
                }
#endif
            }
        }
    }

    return !stateMachine.IsInState<MultiplayerStates::ConnectionInitializing>() &&
            !stateMachine.IsInState<MultiplayerStates::PlayersReconnecting>();
}

void MultiplayerLink::send(u16 packet) {
    bool sent = universal->send(packet);
    if (!sent)
        sendQueue.push(packet);
}

void MultiplayerLink::deactivate(bool sendBye) {
    active = false;
    if (sendBye) {
        universal->sync();
        vsync();
        int attempts = 5;
        bool sent;
        do {
            sent = universal->send(UPDATE_STATE + BYE);
        } while (!sent && --attempts);
    }
    // pass traffic one last time and allow vsync to flush LinkUniversal queue for upstream BYE
    universal->sync();
    vsync();
    universal->deactivate();
    stateMachine.ProcessStateTransitions();
    stateMachine.Shutdown();
    purgeQueues();
    reset();
}

void MultiplayerLink::purgeQueues() {
    clearQueue(sendQueue);
    clearQueue(seedQueue);
}

void MultiplayerLink::UpdateStateMachine() {
    if (!stateMachine.IsInitialized()) {
        stateMachine.SetDebugTraceLevel(hsm::TraceLevel::Diagnostic);
        stateMachine.Initialize<MultiplayerStates::Activated>(this);
    }
    stateMachine.ProcessStateTransitions();
    stateMachine.UpdateStates();
}

void updateRank() {
    if (game->won) {
         rank = 0;
         return;
    }
    rank = 0;
    for (auto i : multiplayerLink->players)
        if (i.second.over)
            rank++;
    rank -= 1;
}


void Command::execute() const {
    int targetId = (value >> 4) & 0xFF;
    int attackValue = value & 0xF;
    switch(command){
        case SEND_ATTACK:
            // Record this attack to judge relative multiplayer rank
            multiplayerLink->enemyPower[playerId.index] += attackValue;
            // Add the attack to the garbage queue if it's for us
            if (targetId == multiplayerLink->universal->currentPlayerId())
                game->addToGarbageQueue(targetId, value & 0xF);
            break;
        case ACK_ATTACK:
            // The player just had garbage placed on their board
            attackAnimationTimers[playerId.index] = animationMax * attackValue;
            break;
        case SEND_ROW:
        case SEND_ROW + 1:
        case SEND_ROW + 2:
            GameScene::UpdateEnemyBoard(command, value, playerId.id);
            break;
    }
}

void clearPlayers() {
    multiplayerLink->players.clear();
}

void removePlayer(u8 pId) {
#ifndef GBA
    multiplayerLink->removePlayer(pId);
#endif
}

void initPlayer(u8 pId) {
    multiplayerLink->initPlayer(pId);
}
