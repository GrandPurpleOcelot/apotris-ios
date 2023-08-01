#include "def.h"
#include "LinkConnection.h"
#include "blockEngine.h"
#include "text.h"
#include <string>
#include "logging.h"

using namespace BlockEngine;

void drawEnemyBoard(int);

u8 enemyBoard[20][10];
OBJ_ATTR* enemyBoardSprite;

int timeoutTimer = 0;
int maxTimeout = 10;

int multiplayerStartTimer = 0;

int attackCheckCooldown = 10;
int attackCheckTimer = 0;

int connected = 0;

int currentScanHeight = 0;

int enemyHeight = 0;

Game *botGame;

int botIncomingHeight = 0;

const int boardTile = 804 - 8;

void handleMultiplayer(int state) {
    #ifdef GBA
    if (!multiplayer)
        return;
    if (multiplayerStartTimer) {
        if (--multiplayerStartTimer == 0) {
            playAgain = true;
        } else {
            linkConnection->send((u16)((1 << 13) + (nextSeed & 0x1fff)));
        }
        return;
    }

    auto linkState = linkConnection->linkState.get();

    int receivedAttack = false;

    u16 data[LINK_MAX_PLAYERS];
    for (int i = 0; i < LINK_MAX_PLAYERS; i++)
        data[i] = 0;

    int incomingHeight = 0;

    if (linkState->isConnected()) {
        timeoutTimer = 0;
        for (u32 i = 0; i < linkState->playerCount; i++)
            while (linkState->hasMessage(i))
                data[i] = linkState->readMessage(i);

        for (u32 i = 0; i < linkState->playerCount; i++) {

            int command = data[i] >> 13;
            int value = data[i] & 0x1fff;

            switch (command) {
            case 0:
                continue;
            case 1:
                playAgain = true;
                nextSeed = value;
                break;
            case 2:
                if (value == 0x123)
                    game->setWin();
                break;
            case 3://receive attack
                game->addToGarbageQueue((value >> 4) & 0xff, value & 0xf);
                receivedAttack = (value >> 4) & 0xff;
                break;
            case 4://acknowledge attack
                game->clearAttack(value & 0xff);
                break;
            case 5:
                for (int i = 0; i < 10; i++)
                    enemyBoard[value >> 10][i] = (value >> i) & 0b1;
                incomingHeight = value >> 10;
                break;
            case 6:
                for (int i = 0; i < 10; i++)
                    enemyBoard[(value >> 10) + 8][i] = (value >> i) & 0b1;
                incomingHeight = (value >> 10) + 8;
                break;
            case 7:
                if ((value >> 10) + 16 > 19)
                    break;
                for (int i = 0; i < 10; i++)
                    enemyBoard[(value >> 10) + 16][i] = (value >> i) & 0b1;
                incomingHeight = (value >> 10) + 16;

                break;
            }
        }
        if (game->lost) {
            if(!state)
                linkConnection->send((u16)(2 << 13) + 0x123);
        } else {
            if (receivedAttack) {
                linkConnection->send((u16)((4 << 13) + receivedAttack));
            } else if (game->attackQueue.size() == 0) {
                if (currentScanHeight < 19)
                    currentScanHeight++;
                else
                    currentScanHeight = 0;

                u16 row = 0;
                for (int i = 0; i < 10; i++)
                    if (game->board[currentScanHeight + 20][i])
                        row += 1 << i;

                if(!state)
                    linkConnection->send((u16)(((5) << 13) + ((currentScanHeight & 0x1f) << 10) + (row & 0x3ff)));
            } else {
                BlockEngine::Garbage atck = game->attackQueue.front();

                linkConnection->send((u16)((3 << 13) + (atck.id << 4) + atck.amount));
            }

            drawEnemyBoard(incomingHeight);
        }
    } else {
        if (timeoutTimer++ == maxTimeout) {
            timeoutTimer = 0;
            game->setWin(); // aprint("no connection.",0,5);
            connected = -1;
            aprint("Connection Lost.", 0, 0);
            multiplayer = false;
        }
    }
    #endif
}

void startMultiplayerGame(int seed) {
    clearSpriteTiles(boardTile, 2,4);

    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            enemyBoard[i][j] = 0;

    delete game;
    game = new Game(4, seed & 0x1fff,bigMode);
    game->setLevel(1);
    game->setTuning(getTuning());
    game->setGoal(100);
    multiplayer = true;

    #ifdef GBA
    linkConnection->send(50);
    #endif
    nextSeed = 0;
}

void drawEnemyBoard(int height) {

    enemyBoardSprite = &obj_buffer[25];
    sprite_set_attr(enemyBoardSprite, ShapeTall, 2, boardTile, 15, 1);
    sprite_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    sprite_enable_affine(enemyBoardSprite, 6, true);
    sprite_set_pos(enemyBoardSprite, 43, 24);
    sprite_set_size(enemyBoardSprite, float2fx(0.5), 6);

    if (height > 19)
        return;

    for (int j = 0; j < 10; j++) {
        if (enemyBoard[height][j])
            setSpritePixel(boardTile, j / 8, height/8, 2, j % 8, height % 8, ((enemyBoard[height][j]) + 1 + savefile->settings.lightMode));
        else
            setSpritePixel(boardTile, j / 8, height/8, 2, j % 8, height % 8, 0);
    }
}

int botClearTimer = 0;

void handleBotGame(){

    if(botIncomingHeight++ > 19){
        botIncomingHeight = 0;
    }

    if(botGame->attackQueue.size()){
        BlockEngine::Garbage atck = botGame->attackQueue.front();
        game->addToGarbageQueue(atck.id,atck.amount);
        botGame->clearAttack(atck.id);
    }

    if(game->attackQueue.size()){
        BlockEngine::Garbage atck = game->attackQueue.front();
        botGame->addToGarbageQueue(atck.id,atck.amount);
        game->clearAttack(atck.id);
    }

    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 10; j++){
            enemyBoard[i][j] = (botGame->board[i+20][j] > 0) * 1;
        }
    }

    drawEnemyBoard(botIncomingHeight);

    if(botGame->clearLock){
        if(++botClearTimer > botGame->maxClearDelay){
            botClearTimer = 0;
            botGame->removeClearLock();
        }
    }

    if(botGame->lost){
        game->won = true;
    }
}
