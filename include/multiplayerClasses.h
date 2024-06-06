#pragma once

#define ENABLE_BOT 0

#include "platform.hpp"

#ifdef GBA

#include "LinkUniversal.hpp"
#include "def.h"
#include "scene.hpp"

#else
#include "LinkWebRTC.hpp"
#endif

#include <memory>
#include <queue>
#include <optional>
#include <utility>
#include "hsm.h"
#include "def.h"
#include "scene.hpp"

// Higher three bits of u16 for commands
#define UPDATE_STATE (2 << 13)
#define SEND_ATTACK 3
#define ACK_ATTACK 4
#define SEND_ROW 5

// Lower 13 bits of u16 for CHANGE_STATE commands
#define BYE 0
#define I_WON 1
#define HELLO 2
#define WELCOME_BACK 3
#define LETS_PLAY 5
#define GAME_OVER 6
#define I_LOST 7

// These are upper bit masks
#define SEND_SEED   0x1000
#define POWER_LEVEL 0x800

using namespace hsm;
const int boardTile = 512 - 32 * 3 - 64 - 8;

void updateRank();

extern void clearEnemyBoard();

class RemotePlayerId {
public:
    u8 id;
    u8 index;

    explicit RemotePlayerId(u8 playerId);
};

class Command {
public:
    int command;
    int value;
    RemotePlayerId playerId;

    Command(RemotePlayerId playerId, u16 packet) : playerId(playerId) {
        command = packet >> 13;
        value = packet & 0x1FFF;
    }

    void execute() const;
};

class Player {
public:
    u8 linkId;
    bool seeded = false;
    bool ready = false;
    bool over = false;
    bool reconnected = false;

    explicit Player(u8 lId) {
        linkId = lId;
    }

    explicit Player() {
        linkId = -1;
    }
};

class MultiplayerLink {
private:
    bool allOverB = false;

    bool allSeeded(u8 playerCount) {
        if (playerCount <= 1 || players.size() <= 1) return false;
        return std::all_of(players.begin(), players.end(),
                           [](std::pair<const unsigned char, Player> i) { return i.second.seeded; });
    }

    bool allOver(u8 playerCount) {
        if (playerCount <= 1 || players.size() <= 1) return false;
        if (playerCount > players.size())
            return allOverB;
        return std::all_of(players.begin(), players.end(),
                           [](std::pair<const unsigned char, Player> i) { return i.second.over; });
    }

    bool allConnected(u8 playerCount) {
        if (playerCount <= 1 || players.size() <= 1) return false;
        return std::all_of(players.begin(), players.end(),
                           [](std::pair<const unsigned char, Player> i) { return i.second.reconnected; });
    }

    void markAllOver(u8 playerCount) {
        allOverB = true;
    }

    friend struct MultiplayerStates;
public:
    LinkUniversal *universal;
    std::map<u8, Player> players;
    StateMachine stateMachine;
    std::queue<u16> sendQueue;
    std::queue<Packet> seedQueue;

    unsigned int playingPlayerCount = 0;
    bool playing = false;
    bool active = false;
    bool playAgain = false;

    u32 enemyPower[4] = {1, 1, 1, 1};

    explicit MultiplayerLink(LinkUniversal *universal) {
        this->universal = universal;
    }

    bool sync();

    void UpdateStateMachine();

    void send(u16 packet);

    void reset() {
        playingPlayerCount = 0;
        playing = false;
        universal->sync();
        nextSeed = 0;
        purgeQueues();
    }

    void initPlayer(u8 i) {
        players[i] = Player(i);
    }

    void resetPlayers() {
        for (auto i: players) {
            players[i.first] = Player(i.first);
        }
    }

    void readyUp() {
        players[universal->currentPlayerId()].ready = true;
    }

    int getReadyCount() {
        int over = 0;
        for (auto &it: players)
            over += it.second.ready;
        return over;
    }

    void purgeQueues();

    void deactivate(bool sendBye);

    void activate();

    [[nodiscard]] std::queue<Packet> readPackets() const {
        std::queue<Packet> queue;
        for (auto player: players) {
            int i = player.first;
            while (universal->canRead(i))
                queue.emplace(i, universal->read(i));
        }
        return queue;
    }

    bool allReady(u8 playerCount) {
        if (playerCount <= 1 || players.size() <= 1) return false;
        return std::all_of(players.begin(), players.end(), [](auto i) {
            return i.second.ready;
        });
    }

#ifndef GBA
    void removePlayer(u8 pId) {
        players.erase(pId);
    }
#endif
};

/**
 * Contains the various outer and inner states that power the multiplayer state machine for networking.
 * Outer states:
 * - Deactivated
 * - Activated
 */
struct MultiplayerStates {

    /**
     * When state is entered, will disable underlying network or wireless hardware automatically.
     * Is the default state when the state machine is not initialized.
     */
    struct Deactivated : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
            if (Owner().universal->isActive()) {
                Owner().universal->deactivate();
            }
        }

        Transition GetTransition() override {
            if (!Owner().active) {
                return NoTransition();
            } else {
                return SiblingTransition<Activated>();
            }
        }
    };

    /**
     * This state manages an active network or GBA wireless connection and calls to the underlying polling mechanism.
     * It awaits the presence of players and will set its own inner states. PlayersPresent is the entry to the inner chain.
     */
    struct Activated : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
            if (!Owner().universal->isActive()) {
                Owner().universal->activate();
            }
            Update();
        }

        void Update() override {
            Owner().universal->sync();
        }

        // Defaults to connection initializing on first entry and when a player disconnects during a live game.
        // Otherwise, proceeds to PlayersPresent and attempts to seed, etc.
        Transition GetTransition() override {
            if (!Owner().universal->isConnected()
                || (Owner().playing && Owner().universal->playerCount() < Owner().playingPlayerCount)) {
                return InnerEntryTransition<ConnectionInitializing>();
            }
            return InnerEntryTransition<PlayersPresent>();
        }
    };

    /**
     * This state captures resetting and/or persisting player connection data for existing or new games before connection
     * and during reconnection.
     */
    struct ConnectionInitializing : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
#ifndef GBA
            if (!Owner().playing) {
                Owner().resetPlayers(); // TODO: GBA needs remove?
            }
#else
            Owner().players.clear();
#endif
        }

        // Waits for correct player count or any connection before marking remote players as "present"
        Transition GetTransition() override {
            if (!Owner().universal->isConnected()
                || (Owner().playing && Owner().universal->playerCount() < Owner().playingPlayerCount)) {
                return NoTransition();
            } else {
                return SiblingTransition<PlayersPresent>();
            }
        }

    };

    /**
     * Handles the unique case of 3+P where only some players are present after disconnect during a live game.
     * Also handles resetting player ready states before a game has started when a disconnect occurs.
     */
    struct PlayersPresent : StateWithOwner<MultiplayerLink> {
        unsigned int seenPlayerCount = 0;

        void OnEnter() override {
            seenPlayerCount = Owner().universal->playerCount();
#ifdef GBA
            for (int i = 0; i < Owner().universal->playerCount(); i++) {
                Owner().initPlayer(i);
            }
#endif
        }

        Transition GetTransition() override {
            if (Owner().playing) {
                if (Owner().playingPlayerCount > Owner().universal->playerCount()) {
                    return SiblingTransition<ConnectionInitializing>();
                } else if (Owner().playingPlayerCount <= Owner().universal->playerCount()) {
                    return InnerEntryTransition<PlayersReconnecting>();
                }
            } else if (seenPlayerCount != Owner().universal->playerCount()) {
                return SiblingTransition<ConnectionInitializing>();
            }
            return InnerEntryTransition<PlayersSeeding>();
        }
    };

    /**
     * Represents the state where some but not all players are synchronized on the host-chosen seed.
     * On entry, the host rolls a seed and transmits it to players. All players await seed messages to confirm synchronization
     * before proceeding, so each client will have the same random state.
     */
    struct PlayersSeeding : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
            if (Owner().universal->currentPlayerId() && nextSeed) {
                Owner().send(UPDATE_STATE + SEND_SEED + nextSeed);
                return;
            }

            if (Owner().universal->currentPlayerId())
                return;

            if (!nextSeed)
                nextSeed = qran() % (SEND_SEED - 1);

            Owner().players[Owner().universal->currentPlayerId()].seeded = true;
            Owner().send(UPDATE_STATE + SEND_SEED + nextSeed);
        }

        void Update() override {
            auto process_queue = [this](std::queue<Packet> &queue) {
                while (!queue.empty()) {
                    Packet packet = queue.front();
                    queue.pop();

                    if ((packet.packet >> 13) != 2)
                        continue;

                    if (packet.packet & SEND_SEED) {
                        if (Owner().universal->currentPlayerId()) {
                            if (!packet.playerId) {
                                nextSeed = packet.packet & (SEND_SEED - 1);
                            } else if (nextSeed && ((packet.packet & (SEND_SEED - 1)) != nextSeed))
                                continue; // bad seed??
                            Owner().players[packet.playerId].seeded = true;
                        } else {
                            if ((packet.packet & (SEND_SEED - 1)) == nextSeed)
                                Owner().players[packet.playerId].seeded = true;
                        }
                    }
                }
            };
            std::queue<Packet> readQueue = Owner().readPackets();
            process_queue(readQueue);

            std::queue<Packet> seedQueue = Owner().seedQueue;
            process_queue(seedQueue);

            if (nextSeed) {
                Owner().players[Owner().universal->currentPlayerId()].seeded = true;
                Owner().send(UPDATE_STATE + SEND_SEED + nextSeed);
            }
        }

        Transition GetTransition() override {
            if (Owner().allSeeded(Owner().universal->playerCount()))
                return SiblingTransition<PlayersSeeded>();
            return NoTransition();
        }
    };

    /**
     * When all players are seeded, we set the playingPlayerCount here, but only if not already in-game.
     * This is how the player count gets determined without confusion before the ready up stage.
     * Relies on the reset operations from earlier stages in the chain.
     */
    struct PlayersSeeded : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
            Owner().playingPlayerCount = Owner().universal->playerCount();
        }

        Transition GetTransition() override {
            if (!Owner().playing) {
                return SiblingTransition<PlayersAwaitReady>();
            } else {
                return NoTransition();
            }
        }
    };

    /**
     * Players send out and await ready-up messages from others in this state, gated by a confirm button in the UI.
     */
    struct PlayersAwaitReady : StateWithOwner<MultiplayerLink> {
        bool sentReady = false;
        int readyTimeout = 120;

        void Update() override {
            std::queue<Packet> queue = Owner().readPackets();

            while (!queue.empty()) {
                Packet packet = queue.front();
                queue.pop();
                auto command = Command(RemotePlayerId(packet.playerId), packet.packet);
                if (command.command != 2) continue;
                if (command.value != LETS_PLAY) continue;
                Owner().players[packet.playerId].ready = true;
            }
            // TODO: Remove bot code
#ifdef GBA
            if (ENABLE_BOT) {
                key_poll();
                if (!key_held(KEY_R))
                    enableBot = true;
                else
                    enableBot = false;
                if (readyTimeout-- <= 0 && qran() % 0xFF == 0)
                    Owner().readyUp();
            }
#else
            /*enableBot = true;
            if (readyTimeout-- <= 0 && qran() % 0xFF == 0)
                Owner().readyUp();*/
#endif
            if (!sentReady && Owner().players[Owner().universal->currentPlayerId()].ready) {
                sentReady = true;
                Owner().send(UPDATE_STATE + LETS_PLAY);
                // Help stragglers catch up (I hope I hope I hope)
                Owner().send(UPDATE_STATE + SEND_SEED + nextSeed);
            }
        }

        void OnExit() override {
            if (!sentReady && Owner().players[Owner().universal->currentPlayerId()].ready)
                Owner().send(UPDATE_STATE + LETS_PLAY);
        }

        Transition GetTransition() override {
            if (Owner().allReady(Owner().playingPlayerCount)) {
                return SiblingTransition<Playing>();
            } else {
                return NoTransition();
            }
        }
    };

    /**
     * The game logic itself, which parses packets of all types into Command classes that have effects when execute()'d
     * It will transition into Won or Lost states depending on flags set by game logic elsewhere in the application.
     */
    struct Playing : StateWithOwner<MultiplayerLink> {
        int echoCounter = 0;

        void OnEnter() override {
            bool wasPlaying = Owner().playing;
            if (!wasPlaying) {
                Owner().playing = true;
                playAgain = Owner().playAgain;
                startMultiplayerGame(nextSeed);
                changeScene(new GameScene());
            }
        }

        void Update() override {
            std::queue<Packet> queue = Owner().readPackets();
            while (!queue.empty()) {
                Packet packet = queue.front();
                queue.pop();

                auto command = Command(RemotePlayerId(packet.playerId), packet.packet);

                if (command.command == 2) {
                    if (command.value == I_LOST)
                        Owner().players[command.playerId.id].over = true;
                    else if (command.value == I_WON) {
                        game->lost = true;
                        Owner().markAllOver(Owner().playingPlayerCount);
                    } else if (command.value == BYE) {
                        u32 newPlayerCount = Owner().playingPlayerCount - 1;
#ifndef GBA
                        removePlayer(packet.playerId);
#endif
                        clearEnemyBoard();
                        Owner().universal->activate();
                        Owner().universal->sync();
                        Owner().playingPlayerCount = newPlayerCount;
                    }
                } else
                    command.execute();
            }

            if (echoCounter++ % 10 == 0)
                Owner().send(UPDATE_STATE + WELCOME_BACK);

            int lostCount = 0;
            for (auto i: Owner().players)
                if (i.second.over)
                    lostCount += 1;
            if (lostCount == Owner().playingPlayerCount - 1)
                game->won = true;
        }

        Transition GetTransition() override {
            if (game->lost) {
                return SiblingTransition<Lost>();
            } else if (game->won) {
                return SiblingTransition<Won>();
            }
            return NoTransition();
        }
    };

    /**
     * When a player has won, they announce it to the network and proceed to the GameOver state.
     */
    struct Won : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
            Owner().send(UPDATE_STATE + I_WON);
            updateRank();
        }

        Transition GetTransition() override {
            Owner().playing = false;
            Owner().resetPlayers();
            Owner().playAgain = true;
            return SiblingTransition<GameOver>();
        }
    };

    /**
     * When a player has lost, they continuously announce it to the network and proceed to the GameOver state when
     * hearing another player has won. This state still parses remote board updates.
     */
    struct Lost : StateWithOwner<MultiplayerLink> {
        unsigned int echoCounter = 0;

        void OnEnter() override {
            Owner().send(UPDATE_STATE + I_LOST);
            updateRank();
            Owner().players[Owner().universal->currentPlayerId()].over = true;
        }

        void Update() override {
            std::queue<Packet> queue = Owner().readPackets();
            while (!queue.empty()) {
                Packet packet = queue.front();
                queue.pop();
                auto command = Command(RemotePlayerId(packet.playerId), packet.packet);
                if (command.command == 2) {
                    if (command.value == I_LOST) {
                        Owner().players[command.playerId.id].over = true;
                    } else if (command.value == I_WON) {
                        Owner().markAllOver(Owner().playingPlayerCount);
                    } else if (command.value == BYE) {
                        u32 newPlayerCount = Owner().playingPlayerCount - 1;
#ifndef GBA
                        removePlayer(packet.playerId);
#endif
                        clearEnemyBoard();
                        Owner().universal->activate();
                        Owner().universal->sync();
                        Owner().playingPlayerCount = newPlayerCount;
                    } else if (command.value & SEND_SEED)
                        Owner().seedQueue.emplace(command.playerId.id,
                                                  (command.command << 13) + command.value);
                } else if (command.command >= SEND_ROW) {
                    command.execute();
                }
                if (echoCounter++ % 60 == 0) {
                    Owner().send(UPDATE_STATE + I_LOST);
                    Owner().send(UPDATE_STATE + WELCOME_BACK);
                }
            }
        }

        Transition GetTransition() override {
            if (!Owner().allOver(Owner().playingPlayerCount)) return NoTransition();
            Owner().playing = false;
            Owner().resetPlayers();
            Owner().playAgain = true;
            return SiblingTransition<GameOver>();
        }
    };

    /**
     * The GameOver state is a temporary state. It exists to announce to any straggling players that are still in-game
     * (perhaps due to dropped network traffic) that the game is considered over and they must reset to re-seed.
     */
    struct GameOver : StateWithOwner<MultiplayerLink> {
        unsigned int echoCounter = 0;

        void OnEnter() override {
            Owner().players[Owner().universal->currentPlayerId()].over = true;
            Owner().send(UPDATE_STATE + GAME_OVER);
            Owner().send(UPDATE_STATE + I_LOST);
        }

        void Update() override {
            std::queue<Packet> queue = Owner().readPackets();

            while (!queue.empty()) {
                Packet packet = queue.front();
                queue.pop();

                auto command = Command(RemotePlayerId(packet.playerId), packet.packet);
                if (command.command != 2) {
                    continue;
                }
                if (command.value == GAME_OVER) {
                    Owner().players[command.playerId.id].over = true;
                } else if (command.value & SEND_SEED) {
                    Owner().seedQueue.emplace(command.playerId.id,
                                              (command.command << 13) + command.value);
                }
            }

            if (echoCounter++ % 10 == 0) {
                Owner().send(UPDATE_STATE + GAME_OVER);
                Owner().send(UPDATE_STATE + I_LOST);
                Owner().send(UPDATE_STATE + WELCOME_BACK);
            }
        }

        void OnExit() override {
            Owner().send(UPDATE_STATE + GAME_OVER);
            Owner().resetPlayers();
            Owner().playing = false;
            Owner().playingPlayerCount = 0;
        }

        Transition GetTransition() override {
            if (Owner().allOver(Owner().playingPlayerCount)) {
                return SiblingTransition<PlayersSeeding>();
            }
            return NoTransition();
        }
    };

    /**
     * When one or more players disconnect(s) from the network, all players wait for each other to echo "welcome back".
     */
    struct PlayersReconnecting : StateWithOwner<MultiplayerLink> {

        void OnEnter() override {
            Owner().send(UPDATE_STATE + WELCOME_BACK);
        }

        void Update() override {
            std::queue<Packet> queue = Owner().readPackets();

            while (!queue.empty()) {
                Packet packet = queue.front();
                queue.pop();

                auto command = Command(RemotePlayerId(packet.playerId), packet.packet);

                if (command.command != 2)
                    continue;

                if (command.value == WELCOME_BACK) {
                    Owner().players[command.playerId.id].reconnected = true;
                    Owner().players[Owner().universal->currentPlayerId()].reconnected = true;
                }
            }

            Owner().send(UPDATE_STATE + WELCOME_BACK);
        }

        Transition GetTransition() override {
            if (Owner().allConnected(Owner().universal->playerCount())) {
                return SiblingTransition<Playing>();
            }
            return NoTransition();
        }
    };
};
extern MultiplayerLink *multiplayerLink;
