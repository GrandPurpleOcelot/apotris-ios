#include "def.h"
#ifdef GBA
#include "LinkUniversal.hpp"
#endif
#include "blockEngine.hpp"
#include "text.h"
#include <string>
#include "logging.h"
#include "rumble.h"
#include "scene.hpp"

using namespace BlockEngine;

void drawEnemyBoard();
u8 weightedRandomPlayerSelection();
u8 getInitialId(u8 realId);

u8 enemyBoard[4][20][10];
u32 enemyPower[4] = { 1, 1, 1, 1 };
u8 currentPlayerId = -1;
int initialPlayerCount = -1;
// Current to initial
std::map<u8, u8> currentIdToInitialIdLookup = {
    { 0, 0 },
    { 1, 1 },
    { 2, 2 },
    { 3, 3 },
    { 4, 4 }
};

class BoardUpdate{
public:
    int row = 0;
    int playerId = 0;

    BoardUpdate(int r, int id){
        row = r;
        playerId = id;
    }
};

std::map<u8, u8> playerToBoardIndex;
std::vector<BoardUpdate> enemyBoardUpdates;

std::map<u8, bool> loseIndex = {
    { 0, false },
    { 1, false },
    { 2, false },
    { 3, false },
    { 4, false }
};

OBJ_ATTR* enemyBoardSprite;

bool multiplayerStart = false;
int rank = 0;
int connected = 0;
int currentScanHeight = 0;

Game *botGame;
int botIncomingHeight = 0;

const int boardTile = 512 - 32 * 3 - 64;

#define PLAY_AGAIN 1
#define CHANGE_STATE 2
#define SEND_ATTACK 3
#define SEND_ROW 5
#define ENCODE(x) ((x) << 13)

// returns true if we should terminate the game
int GameScene::handleMultiplayer(int state) {
    #ifdef GBA
    if (!multiplayer)
        return 0;

    // Update remote state and fetch incoming data
    linkConnection->sync();
    u8 currentPlayerCount = linkConnection->playerCount();
    currentPlayerId = linkConnection->currentPlayerId();

    // Capture initial player count
    if (initialPlayerCount < 0)
        initialPlayerCount = currentPlayerCount;

    // Exit early for a later update if bad state (e.g. some player disconnects)
    if (currentPlayerCount < initialPlayerCount) {
        // If reconnect returns false, we got a connection back with all players
        if (!reconnect()) {
            // Get potential new playerId
            linkConnection->sync();
            u8 newPlayerId = linkConnection->currentPlayerId();
            // Send out initial ID to map to new if changed
            if (currentPlayerId != newPlayerId) {
                linkConnection->send(ENCODE(CHANGE_STATE) + currentPlayerId);
                currentIdToInitialIdLookup[newPlayerId] = currentPlayerId;
                currentPlayerId = newPlayerId;
            }
            return 2;
        } else {
            return 1;
        }
    }

    // Game start
    if (multiplayerStart) {
        multiplayerStart = false;
        loseIndex = { {0, false}, {1, false}, {2, false}, {3, false}, {4, false} };
        for (auto & i : enemyPower)
            i = 1;
        playAgain = true;
        rank = 0;
        linkConnection->send((u16) (ENCODE(PLAY_AGAIN) + (nextSeed & 0x1fff)));
        return 0;
    }

    // Handle remote player updates
    for (int remotePlayerId = 0; remotePlayerId < initialPlayerCount; remotePlayerId++) {
        // Skip self
        if (remotePlayerId == currentPlayerId)
            continue;

        // Process remote player's packets
        while (linkConnection->canRead(remotePlayerId))
            ProcessMultiplayerPacket(linkConnection->read(remotePlayerId), remotePlayerId);

        if (game->lost) {
            if (!state) {
                linkConnection->send((u16) ENCODE(CHANGE_STATE) + 0x123);
                updateRank();
            }
        } else {
            // Attacks take priority over block state
            if (!game->attackQueue.empty()) {
                // Grab next attack
                BlockEngine::Garbage atck = game->attackQueue.front();
                // Pop it out of pending attack queue
                game->attackQueue.pop_front();
                // Encode and send out attack
                u8 targetId = getInitialId(weightedRandomPlayerSelection());
                linkConnection->send((u16) (ENCODE(SEND_ATTACK) + (targetId << 4) + atck.amount));
            } else {
                // Rotate over the 20 rows
                if (currentScanHeight < 19)
                    currentScanHeight++;
                else
                    currentScanHeight = 0;

                // Store row as each bit on or off for the presence of each block
                u16 row_bitfield = 0;
                for (int i = 0; i < 10; i++)
                    if (game->board[currentScanHeight + 20][i])
                        row_bitfield += 1 << i;

                if (!state)
                    linkConnection->send(
                            (u16) (ENCODE(SEND_ROW) + ((currentScanHeight & 0x1f) << 10) + (row_bitfield & 0x3ff)));
            }
        }
    }

    drawEnemyBoard();
    #endif

    return 0;
}

u8 weightedRandomPlayerSelection() {
#ifdef GBA
    // Skip 2P mode
    if (linkConnection->playerCount() == 2)
        return 0;

    // Calculate the total attack power
    u32 totalAttackPower = 0;
    for (int i = 0; i < initialPlayerCount; ++i) {
        totalAttackPower += enemyPower[i];
    }

    // Pick randomly if no power
    if (!totalAttackPower)
        return qran() % initialPlayerCount;

    // Generate a random number between 0 and totalAttackPower
    u32 r = qran() % totalAttackPower;

    // Determine which player is selected
    u32 accumulatedPower = 0;
    for (int i = 0; i < initialPlayerCount; ++i) {
        accumulatedPower += enemyPower[i];
        if (r < accumulatedPower)
            return i; // Return the player ID
    }

    // This point should never be reached if the enemyBoard is not empty
#endif
    return -1; // Return an invalid player ID
}

void GameScene::updateRank() {
    rank = 0;
    for (int playerId = 0; playerId < initialPlayerCount; playerId++)
        if (!loseIndex[playerId])
            rank++;
}

int getEnemyIndex(u8 id) {
    auto realId = currentIdToInitialIdLookup[id];
    if(realId > currentPlayerId)
        realId -= 1;
    return realId;
}

// Never returns own ID
u8 getInitialId(u8 realId) {
    std::map<u8, u8> initialIdToCurrentIdLookup;
    for (const auto& kv : currentIdToInitialIdLookup) {
        initialIdToCurrentIdLookup[kv.second] = kv.first;
    }
    if (realId >= currentPlayerId) {
        realId += 1;
    }
    auto it = initialIdToCurrentIdLookup.find(realId);
    if (it != initialIdToCurrentIdLookup.end()) {
        return it->second;
    } else {
        // This should never happen
        return 255;
    }
}

// Function to process multiplayer packets
void GameScene::ProcessMultiplayerPacket(u16 data, u8 remotePlayerId) {
    // Extract the command from the upper 3 bits of the data
    int command = data >> 13;
    // Extract the value from the lower 13 bits of the data
    int value = data & 0x1FFF;

    // If the command is 1, restart the game with a new seed
    if (command == PLAY_AGAIN) {
        playAgain = true;
        nextSeed = value;
    }
    // Command 2 means special updates
    else if (command == CHANGE_STATE)
        // If the value is 0x123, an opponent has lost
        if (value == 0x123) {
            loseIndex[currentIdToInitialIdLookup[remotePlayerId]] = true;
            if (game->lost)
                return;
            updateRank();
            if(rank == 1)
                game->setWin();
        }
        // Otherwise the value is an old playerId, and we were in a reconnect state, so update
        else
            currentIdToInitialIdLookup[remotePlayerId] = value;
    // If the command is between 3 and 7, process the game update
    else if (command >= SEND_ATTACK && command <= 7)
        ProcessGameUpdate(command, value, remotePlayerId);
}

// Function to process game updates
void GameScene::ProcessGameUpdate(int command, int value, u8 remotePlayerId) {
    // If the command is 3, receive an attack
    if (command == SEND_ATTACK) {
        int targetId = (value >> 4) & 0xFF;
        int attackValue = value & 0xF;
        // Record this attack to judge relative multiplayer rank
        enemyPower[getEnemyIndex(remotePlayerId)] += attackValue;
        // Add the attack to the garbage queue if it's for us
        if (targetId == currentPlayerId)
            game->addToGarbageQueue(targetId, value & 0xF);
    }
    // If the command is between 5 and 7, update the enemy board
    else
        UpdateEnemyBoard(command, value, remotePlayerId);
}

// Function to update the enemy board
void GameScene::UpdateEnemyBoard(int command, int value, u8 remotePlayerId) {
    // Calculate the base height based on the command
    int baseHeight = (command - 5) * 8;
    // Calculate the row to be updated
    int row = (value >> 10) + baseHeight;
    // If the row is within the board's height
    if (row < 20) {
        int id = getEnemyIndex(remotePlayerId);
        // Loop through each column in the row
        for (int j = 0; j < 10; j++)
            // Update the cell in the enemy board based on the value
            enemyBoard[id][row][j] = (value >> j) & 0x1;

        enemyBoardUpdates.emplace_back(row,id);
    }
}

void startMultiplayerGame(int seed) {
    clearSpriteTiles(boardTile, 2,4);

    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            for (auto & k : enemyBoard)
                k[i][j] = 0;

    delete game;
    game = new Game(BATTLE, seed & 0x1fff,false);

    if(previousGameOptions == nullptr)
        previousGameOptions = new BlockEngine::Options();

    game->setOptions(*previousGameOptions);
    game->setLevel(1);
    game->setGoal(100);
    multiplayer = true;

    #ifdef GBA
    linkConnection->send(50);
    #endif

    nextSeed = 0;
}

void drawEnemyBoard() {
    if(game->won || game->lost){
        return;
    }

    enemyBoardSprite = &obj_buffer[25];
    sprite_set_attr(enemyBoardSprite, ShapeSquare, 3, boardTile, 15, 1);
    sprite_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    sprite_enable_affine(enemyBoardSprite, 6, true);
    sprite_set_pos(enemyBoardSprite, 43 - 32, 20 - 32);
    sprite_set_size(enemyBoardSprite, int2fx(1), 6);

    const int margin = 2;

    for(auto const& update : enemyBoardUpdates){
        const int offsetX = (update.playerId % 2) * (10 + margin);
        const int offsetY = (update.playerId / 2) * (20 + margin);

        const int y = update.row + offsetY;

        for (int j = 0; j < 10; j++) {
            const int x = j + offsetX;

            if (enemyBoard[update.playerId][update.row][j])
                setSpritePixel(boardTile, x / 8, y/8, 8, x % 8, y % 8,
                            ((enemyBoard[update.playerId][update.row][j]) + 1 + savefile->settings.lightMode));
            else
                setSpritePixel(boardTile, x / 8, y/8, 8, x % 8, y % 8, 0);
        }
    }

    enemyBoardUpdates.clear();
}

int botClearTimer = 0;

void handleBotGame(){

    if(botIncomingHeight++ > 19){
        botIncomingHeight = 0;
    }

    if(!botGame->attackQueue.empty()){
        BlockEngine::Garbage atck = botGame->attackQueue.front();
        game->addToGarbageQueue(atck.id,atck.amount);
        botGame->clearAttack(atck.id);
    }

    if(!game->attackQueue.empty()){
        BlockEngine::Garbage atck = game->attackQueue.front();
        botGame->addToGarbageQueue(atck.id,atck.amount);
        game->clearAttack(atck.id);
    }

    for(int i = 0; i < 20; i++){
        for(int j = 0; j < 10; j++){
            enemyBoard[0][i][j] = (botGame->board[i+20][j] > 0) * 1;
        }
    }

    drawEnemyBoard();

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

    for(auto & wordSprite : wordSprites)
        wordSprite.hide();

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

    while(timer++ < 30 || (linkConnection->playerCount() != initialPlayerCount && closed())){
        vsync();
        key_poll();

        if(timer == 120){
            naprint(exit, 120 - getVariableWidth(exit) / 2, 144);
        }

        if(timer >= 120){
            if(key_hit(k.confirm)){
                multiplayer = false;
                linkConnection->deactivate();
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

        showSprites(128);
        linkConnection->sync();
    }

    clearTilemap(27);
    clearTilemap(26);
    buildBG(3, 0, 27, 0, 1, true);

    enableBlend(prevBld);

    for(auto & wordSprite : wordSprites)
        wordSprite.hide();

    for(auto & cursorSprite : cursorSprites)
        sprite_hide(cursorSprite);

    drawFrame(0);
    if(game->zoneTimer)
        frameSnow(0);

    showBackground(0);
    resetSmallText();
    clearText();
    setSmallTextArea(110, 3, 7, 9, 10);
    showText();

    if(!(game->won || game->lost))
        countdown();

    resumeSong();

    #endif

    return false;
}
