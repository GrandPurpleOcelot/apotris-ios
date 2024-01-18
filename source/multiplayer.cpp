#include "def.h"
#include "LinkUniversal.hpp"
#include "blockEngine.hpp"
#include "text.h"
#include <string>
#include "logging.h"
#include "rumble.h"
#include "scene.hpp"

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

const int boardTile = 512 - 32 * 3 - 8;

bool GameScene::handleMultiplayer(int state) {
    #ifdef GBA
    if (!multiplayer)
        return false;

    linkUniversal->sync();
    u8 currentPlayerId = linkUniversal->currentPlayerId();
    u8 remotePlayerId = !currentPlayerId;
    u8 playerCount = linkUniversal->playerCount();

    if (linkUniversal->isConnected() && playerCount == 2) {
        if (multiplayerStartTimer) {
            if (--multiplayerStartTimer == 0) {
                playAgain = true;
            } else {
                linkUniversal->send((u16)((1 << 13) + (nextSeed & 0x1fff)));
                linkUniversal->waitFor(remotePlayerId);
            }
            return false;
        }

        int receivedAttack = false;
        int incomingHeight = 0;

        timeoutTimer = 0;
        u16 data = 0;
        if (linkUniversal->canRead(remotePlayerId))
            data = linkUniversal->read(remotePlayerId);
        ProcessMultiplayerPacket(data, receivedAttack, incomingHeight);

        if (game->lost) {
            if(!state) {
                linkUniversal->send((u16) (2 << 13) + 0x123);
            }
        } else {
            if (receivedAttack) {
                linkUniversal->send((u16)((4 << 13) + receivedAttack));
            } else if (game->attackQueue.empty()) {
                if (currentScanHeight < 19)
                    currentScanHeight++;
                else
                    currentScanHeight = 0;

                u16 row = 0;
                for (int i = 0; i < 10; i++)
                    if (game->board[currentScanHeight + 20][i])
                        row += 1 << i;

                if(!state)
                    linkUniversal->send((u16) (((5) << 13) + ((currentScanHeight & 0x1f) << 10) + (row & 0x3ff)));
            } else {
                BlockEngine::Garbage atck = game->attackQueue.front();

                linkUniversal->send((u16)((3 << 13) + (atck.id << 4) + atck.amount));
            }

            drawEnemyBoard(incomingHeight);
        }
    } else {
        if(reconnect()){
            return true;
        }
    }
    #endif

    return false;
}

// Function to process multiplayer packets
void GameScene::ProcessMultiplayerPacket(u16 data, int &receivedAttack, int &incomingHeight) {
    // Extract the command from the upper 3 bits of the data
    int command = data >> 13;
    // Extract the value from the lower 13 bits of the data
    int value = data & 0x1FFF;

    // If the command is 1, restart the game with a new seed
    if (command == 1) {
        playAgain = true;
        nextSeed = value;
    }
    // If the command is 2 and the value is 0x123, the opponent has lost
    else if (command == 2 && value == 0x123)
        game->setWin();
    // If the command is between 3 and 7, process the game update
    else if (command >= 3 && command <= 7)
        ProcessGameUpdate(command, value, receivedAttack, incomingHeight);
}

// Function to process game updates
void GameScene::ProcessGameUpdate(int command, int value, int &receivedAttack, int &incomingHeight) {
    // If the command is 3, receive an attack
    if (command == 3) {
        int attackValue = (value >> 4) & 0xFF;
        // Add the attack to the garbage queue
        game->addToGarbageQueue(attackValue, value & 0xF);
        // Set the received attack value
        receivedAttack = attackValue;
    }
    // If the command is 4, acknowledge the attack
    else if (command == 4)
        game->clearAttack(value & 0xFF);
    // If the command is between 5 and 7, update the enemy board
    else
        UpdateEnemyBoard(command, value, incomingHeight);
}

// Function to update the enemy board
void GameScene::UpdateEnemyBoard(int command, int value, int &incomingHeight) {
    // Calculate the base height based on the command
    int baseHeight = (command - 5) * 8;
    // Calculate the row to be updated
    int row = (value >> 10) + baseHeight;
    // If the row is within the board's height
    if (row < 20) {
        // Loop through each column in the row
        for (int j = 0; j < 10; j++)
            // Update the cell in the enemy board based on the value
            enemyBoard[row][j] = (value >> j) & 0x1;
        // Set the incoming height to the updated row
        incomingHeight = row;
    }
}

void startMultiplayerGame(int seed) {
    clearSpriteTiles(boardTile, 2,4);

    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            enemyBoard[i][j] = 0;

    delete game;
    game = new Game(BATTLE, seed & 0x1fff,false);

    if(previousGameOptions == nullptr)
        previousGameOptions = new BlockEngine::Options();

    game->setOptions(*previousGameOptions);
    game->setLevel(1);
    game->setGoal(100);
    multiplayer = true;

    #ifdef GBA
    linkUniversal->send(50);
    #endif

    nextSeed = 0;
}

void drawEnemyBoard(int height) {
    if(game->won || game->lost){
        return;
    }

    enemyBoardSprite = &obj_buffer[25];
    sprite_set_attr(enemyBoardSprite, ShapeTall, 2, boardTile, 15, 1);
    sprite_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    sprite_enable_affine(enemyBoardSprite, 6, true);
    sprite_set_pos(enemyBoardSprite, 43, 20);
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

bool GameScene::reconnect(){
    #ifdef GBA
    pauseSong();

    resetSmallText();
    clearText();

    for(int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i].hide();

    int prevBld = blendInfo;
    enableBlend((1 << 6) + (0b11101 << 9) + (1 << 3));
    setTiles(27,0,32*32,tileBuild(34, false, false, 0));
    buildBG(3, 0, 27, 0, 0, 0);
    clearTilemap(25);

    drawFrame(1);
    if(game->zoneTimer)
        frameSnow(1);

    setLayerScroll(2, 0, 0);

    //hide Sprites
    hideMinos();
    sprite_hide(&obj_buffer[23]); //hide meter
    sprite_hide(&obj_buffer[24]); //hide finesse combo counter
    sprite_hide(&obj_buffer[25]); //hide enemyBoard
    for(int i = 0; i < 3; i++)
        sprite_hide(&obj_buffer[16+i]);

    showSprites(128);

    #ifdef GBA
    rumble_set_state(rumble_stop);
    #endif

    int cursorFloat = 0;
    OBJ_ATTR * cursorSprites[2];

    loadSpriteTiles(16*7, blockSprite, 1, 1);
    for(int i = 0; i < 2; i++){
        cursorSprites[i] = &obj_buffer[1 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 0);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_hide(cursorSprites[i]);
    }

    MenuKeys k = savefile->settings.menuKeys;

    naprint("Connection Lost",8,8);

    std::string str = "Attempting to Reconnect...";
    naprint(str, 120 - getVariableWidth(str) / 2, 80);

    int timer = 0;

    std::string exit = "Exit";

    u8 playerCount = linkUniversal->playerCount();
    while(!(linkUniversal->isConnected() && playerCount == 2) && closed()){
        vsync();
        key_poll();

        if(timer == 120){
            naprint(exit, 120 - getVariableWidth(exit) / 2, 144);
        }

        if(timer >= 120){
            if(key_hit(k.confirm)){
                multiplayer = false;
                linkUniversal->deactivate();
                connected = -1;

                sfx(SFX_MENUCANCEL);
                enableBlend(prevBld);
                playSongRandom(0);
                buildBG(3, 0, 27, 0, 1, true);
                changeScene(new MainMenuScene(), Transitions::FADE);
                return true;
            }

            cursorFloat += 6;
            if(cursorFloat >= 512)
                cursorFloat = 0;
            int offset = (sinLut(cursorFloat) * 2) >> 12;
            FIXED scale = float2fx((1.0 - ((float) 0.1 * offset)));

            for (int i = 0; i < 2; i++) {
                sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 0);
                sprite_enable_affine(cursorSprites[i], i, true);
                sprite_set_size(cursorSprites[i], scale, i);

                int x = 240 / 2 - ((getVariableWidth(exit) + 8) / 2 + offset + 4) * ((i) ? -1 : 1) - 8;

                sprite_set_pos(cursorSprites[i], x, 144 - 5);
            }
        }

        linkUniversal->sync();
        playerCount = linkUniversal->playerCount();
        timer++;

        showSprites(128);
    }

    clearTilemap(27);
    clearTilemap(26);
    buildBG(3, 0, 27, 0, 1, true);

    enableBlend(prevBld);

    for(int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i].hide();

    for(int i = 0; i < 2; i++)
        sprite_hide(cursorSprites[i]);

    drawFrame(0);
    if(game->zoneTimer)
        frameSnow(0);

    showBackground(0);
    resetSmallText();
    clearText();
    setSmallTextArea(110, 3, 7, 9, 10);
    showText();

    countdown();

    resumeSong();

    #endif

    return false;
}
