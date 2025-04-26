#include "def.h"

#ifdef GBA
#include "LinkUniversal.hpp"
#endif
#include "blockEngine.hpp"
#include "logging.h"
#include "multiplayerClasses.h"
#include "rumble.h"
#include "scene.hpp"
#include "text.h"
#include <queue>
#include <string>

using namespace BlockEngine;

void drawEnemyBoard();
int weightedRandomPlayerSelection();

u8 enemyBoard[4][20][10];
u8 attackAnimationTimers[4];

u32 ownPower = 1;

class BoardUpdate {
public:
    int row = 0;
    RemotePlayerId playerId;

    BoardUpdate(int r, RemotePlayerId playerId) : playerId(playerId) {
        row = r;
    }
};

std::vector<BoardUpdate> enemyBoardUpdates;

OBJ_ATTR* enemyBoardSprite;

bool multiplayerStart = false;
int rank = 0;
int currentScanHeight = 0;

Game* botGame;
int botIncomingHeight = 0;

#define ENCODE(x) ((x) << 13)

void clearEnemyBoard() {
    memset32_fast((u32*)enemyBoard, 0, sizeof(enemyBoard) / 4);
    clearSpriteTiles(boardTile, 8, 8);

    for (auto& timer : attackAnimationTimers)
        timer = 0;
}

// returns true if we should terminate the game
int GameScene::handleMultiplayer(bool duringGame) {
    if (!multiplayer)
        return 0;

    // Update remote state and fetch incoming data
    bool playersPresent = multiplayerLink->sync();

    // Exit early for a later update if bad state (e.g. some player disconnects)
    if (!playersPresent) {
        // If reconnect returns false, we got a connection back with all players
        if (reconnect()) {
            // Get potential new playerId
            multiplayerLink->sync();
            // Send out power and ID or lose state to map to new if changed
            u32 power = ownPower;
            if (game->lost)
                power = 0; // we lost
            multiplayerLink->send(UPDATE_STATE + POWER_LEVEL + power);
            return 0;
        } else {
            return 1;
        }
    }

    if (multiplayerStart)
        return 0;

    if (!game->lost) {
        // Attacks take priority over block state
        if (!game->attackQueue.empty()) {
            // Grab next attack
            BlockEngine::Garbage atck = game->attackQueue.front();
            // Pop it out of pending attack queue
            game->attackQueue.pop_front();
            // Encode and send out attack
            u8 targetId = weightedRandomPlayerSelection();
            multiplayerLink->send(
                (u16)(ENCODE(SEND_ATTACK) + (targetId << 4) + atck.amount));
            ownPower += atck.amount;
        } else {
            if (duringGame) {
                int rowsToSend = 1;
                u16 packet;
                bool sent;
                // Note that board updates do not use the send queue as they are
                // best-effort
                do {
                    // Rotate over the 20 rows
                    if (currentScanHeight < 19)
                        currentScanHeight++;
                    else
                        currentScanHeight = 0;

                    // Store row as each bit on or off for the presence of each
                    // block
                    u16 row_bitfield = 0;
                    for (int i = 0; i < 10; i++)
                        if (game->board[currentScanHeight + 20][i])
                            row_bitfield += 1 << i;

                    packet = (u16)(ENCODE(SEND_ROW) +
                                   ((currentScanHeight & 0x1f) << 10) +
                                   (row_bitfield & 0x3ff));
                    sent = multiplayerLink->universal->send(packet);

                } while (rowsToSend-- && sent); // avoid flooding
            }
        }
    }

    drawEnemyBoard();

    return 0;
}

int weightedRandomPlayerSelection() {
    auto loseIndex = multiplayerLink->players;
    // Skip 2P mode
    if (multiplayerLink->playingPlayerCount == 2) {
        if (0 == multiplayerLink->universal->currentPlayerId()) {
            return 1; // choose the other player if current player is selected.
        }
        return 0;
    }

    // Calculate the total attack power
    u32 totalAttackPower = 0;
    for (int i = 0; i < multiplayerLink->playingPlayerCount; ++i) {
        if (i == multiplayerLink->universal->currentPlayerId())
            continue; // check if player is current one
        auto id = RemotePlayerId(i);
        if (loseIndex[id.id].over)
            continue;
        totalAttackPower += multiplayerLink->enemyPower[id.index];
    }

    // Pick randomly if no power
    if (!totalAttackPower) {
        u8 randomPlayer =
            static_cast<u8>((int)randNext() % multiplayerLink->playingPlayerCount);
        while (randomPlayer == multiplayerLink->universal->currentPlayerId() &&
               loseIndex[randomPlayer].over) {
            // get another random value if it equals the current player's id or
            // they lost
            randomPlayer =
                static_cast<u8>((int)randNext() % multiplayerLink->playingPlayerCount);
        }
        return randomPlayer;
    }

    // Generate a random number between 0 and totalAttackPower
    u32 r = randNext() % totalAttackPower;

    // Determine which player is selected
    u32 accumulatedPower = 0;
    for (int i = 0; i < multiplayerLink->playingPlayerCount; ++i) {
        if (i == multiplayerLink->universal->currentPlayerId())
            continue; // check if player is current one
        auto id = RemotePlayerId(i);
        if (loseIndex[id.id].over)
            continue;
        accumulatedPower += multiplayerLink->enemyPower[id.index];
        if (r < accumulatedPower) {
            return i; // Return the player ID
        }
    }

    // This point should never be reached if the enemyBoard is not empty
    // Try to find an alternative player than current one
    for (int i = 0; i < multiplayerLink->playingPlayerCount; ++i) {
        if (i != multiplayerLink->universal->currentPlayerId() &&
            loseIndex[i].over == false) {
            return i;
        }
    }

    return -1; // Return an invalid player ID if no other player is found
}

void acknowledgeGarbage(BlockEngine::Game* sourceGame,
                        BlockEngine::Garbage garbage) {
    if (multiplayer) {
        multiplayerLink->send(ENCODE(ACK_ATTACK) + (garbage.id << 4) +
                              garbage.amount);
    } else {
        if (sourceGame == botGame) {
            attackAnimationTimers[0] = animationMax * garbage.amount;
        }
    }
}

// Function to update the enemy board
void GameScene::UpdateEnemyBoard(int command, int value, u8 linkPlayerId) {
    auto remotePlayerId = RemotePlayerId(linkPlayerId);
    // Calculate the base height based on the command
    int baseHeight = (command - 5) * 8;
    // Calculate the row to be updated
    int row = (value >> 10) + baseHeight;
    // If the row is within the board's height
    if (row < 20) {
        // Loop through each column in the row
        for (int j = 0; j < 10; j++)
            // Update the cell in the enemy board based on the value
            enemyBoard[remotePlayerId.index][row][j] = (value >> j) & 0x1;

        enemyBoardUpdates.emplace_back(row, remotePlayerId);
    }
}

void startMultiplayerGame(int seed) {
    clearSpriteTiles(boardTile, 2, 4);
    clearText();
    if (playAgain)
        clearSmallText();

    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            for (auto& k : enemyBoard)
                k[i][j] = 0;

    delete game;
    game = new Game(BATTLE, seed & (SEND_SEED - 1), false);

    if (previousGameOptions == nullptr)
        previousGameOptions = new BlockEngine::Options();

    game->setOptions(*previousGameOptions);
    game->setTuning(getTuning());
    game->setLevel(1);
    game->setGoal(250);
    multiplayer = true;
    clearEnemyBoard();
    game->won = false;

    nextSeed = 0;
    multiplayerStart = true;
    botThinkingSpeed = 8 + multiplayerLink->universal->currentPlayerId();
    for (auto& i : multiplayerLink->enemyPower)
        i = 1;
    ended = false;
    rank = 0;
    ownPower = 1;
    delete testBot;
    if (enableBot) {
        testBot = new BlockEngine::Bot(game);
    }
}

void drawEnemyBoard() {
    if (!multiplayerLink->stateMachine.IsInState<MultiplayerStates::Lost>() &&
        (game->won || game->lost)) {
        return;
    }

    bool zoom = false;
    if (multiplayer) {
        if (multiplayerLink->playingPlayerCount <= 2) {
            zoom = true;
        }
    } else {
        zoom = true;
    }

    int x = 43 - 32;
    int y = 20 - 32;

    if (zoom) {
        x += 32;
        y += 32;
    }

    enemyBoardSprite = &obj_buffer[25];
    sprite_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    sprite_set_attr(enemyBoardSprite, ShapeSquare, 3, boardTile, 15, 1);
    sprite_enable_affine(enemyBoardSprite, 25, true);
    sprite_set_pos(enemyBoardSprite, x, y);
    sprite_set_size(enemyBoardSprite, int2fx(1) >> zoom, 25);

    const int margin = 3;

    int count = 1;
    if (multiplayer) {
        count = multiplayerLink->playingPlayerCount - 1;
    }

    for (int i = 0; i < count; i++) {
        OBJ_ATTR* boardFrame = &obj_buffer[26 + i];

        const int offsetX = (i % 2) * (10 + margin) + (25) * !zoom;
        const int offsetY = (i / 2) * (20 + margin) + (17) * !zoom;

        sprite_set_attr(boardFrame, ShapeTall, 2, boardTile + 64,
                        14 + (attackAnimationTimers[i] > 0), 1);
        sprite_unhide(boardFrame, ATTR0_AFF_DBL);
        sprite_enable_affine(boardFrame, 6, true);
        sprite_set_pos(boardFrame, x + offsetX - 2, y + offsetY - 2);
        sprite_set_size(boardFrame, int2fx(1) >> zoom, 6);

        if (attackAnimationTimers[i])
            attackAnimationTimers[i]--;
    }

    std::map<u8, bool> lost;
    for (auto i = 0; i < multiplayerLink->playingPlayerCount; i++) {
        if (i == multiplayerLink->universal->currentPlayerId())
            continue;
        auto id = RemotePlayerId(i);
        if (multiplayerLink->players[i].over) {
            const int offsetX = (id.index % 2) * (10 + margin);
            const int offsetY = (id.index / 2) * (20 + margin);
            for (int j = 0; j < 10; j++) {
                for (int yy = 0; yy < 20; yy++) {
                    auto yyy = yy + offsetY;
                    auto color = 2 + savefile->settings.lightMode;
                    const int xx = j + offsetX;
                    setSpritePixel(boardTile, xx / 8, yyy / 8, 8, xx % 8,
                                   yyy % 8, color);
                }
            }
        }
    }

    for (auto const& update : enemyBoardUpdates) {
        const int offsetX = (update.playerId.index % 2) * (10 + margin);
        const int offsetY = (update.playerId.index / 2) * (20 + margin);

        const int yy = update.row + offsetY;

        for (int j = 0; j < 10; j++) {
            const int xx = j + offsetX;

            if (enemyBoard[update.playerId.index][update.row][j]) {
                auto color =
                    (enemyBoard[update.playerId.index][update.row][j]) + 1 +
                    savefile->settings.lightMode;
                setSpritePixel(boardTile, xx / 8, yy / 8, 8, xx % 8, yy % 8,
                               color);
            } else
                setSpritePixel(boardTile, xx / 8, yy / 8, 8, xx % 8, yy % 8, 0);
        }
    }

    enemyBoardUpdates.clear();
}

int botClearTimer = 0;

void handleBotGame() {

    if (botIncomingHeight++ > 19) {
        botIncomingHeight = 0;
    }

    if (!botGame->attackQueue.empty()) {
        BlockEngine::Garbage atck = botGame->attackQueue.front();
        game->addToGarbageQueue(atck.id, atck.amount);
        botGame->clearAttack(atck.id);
    }

    if (!game->attackQueue.empty()) {
        BlockEngine::Garbage atck = game->attackQueue.front();
        botGame->addToGarbageQueue(atck.id, atck.amount);
        game->clearAttack(atck.id);
    }

    const auto id = RemotePlayerId(1);

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            enemyBoard[id.index][i][j] = (botGame->board[i + 20][j] > 0) * 1;
        }
    }

    enemyBoardUpdates.emplace_back(botIncomingHeight, id);
    drawEnemyBoard();

    if (botGame->clearLock) {
        if (++botClearTimer > botGame->maxClearDelay) {
            botClearTimer = 0;
            botGame->removeClearLock();
        }
    }

    if (botGame->lost) {
        game->won = true;
    }
}

// returns true on reconnect
bool GameScene::reconnect() {
    pauseSong();
    game->liftKeys();

    resetSmallText();
    clearText();
    int prevBld = blendInfo;

    for (auto& wordSprite : wordSprites)
        wordSprite.hide();

    enableBlend((1 << 6) + (0b11101 << 9) + (1 << 3));
    setTiles(27, 0, 32 * 32, tileBuild(34, false, false, 0));
    buildBG(3, 0, 27, 0, 0, 0);
    clearTilemap(25);

    drawFrame(1);
    if (game->zoneTimer)
        frameSnow(1);

    setLayerScroll(2, 0, 0);

    // hide Sprites
    hideMinos();
    sprite_hide(&obj_buffer[23]); // hide meter
    sprite_hide(&obj_buffer[24]); // hide finesse combo counter
    sprite_hide(&obj_buffer[25]); // hide enemyBoard
    for (int i = 0; i < 3; i++)
        sprite_hide(&obj_buffer[16 + i]);

    showSprites(128);

    rumbleStop();

    int cursorFloat = 0;
    OBJ_ATTR* cursorSprites[2];

    loadSpriteTiles(16 * 7, blockSprite, 1, 1);
    for (int i = 0; i < 2; i++) {
        cursorSprites[i] = &obj_buffer[1 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 0);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_hide(cursorSprites[i]);
    }

    MenuKeys k = savefile->settings.menuKeys;

    naprint("Connection Lost", 8, 8);

    std::string str = "Attempting to Reconnect...";
    naprint(str, 120 - getVariableWidth(str) / 2, 80);

    int timer = 0;

    std::string exit = "Exit";

    while (!multiplayerLink->sync() && closed()) {
        vsync();
        key_poll();

        if (!multiplayerLink->active) {
            multiplayer = false;
            enableBlend(prevBld);
            playSongRandom(0);
            buildBG(3, 0, 27, 0, 1, true);
            exceptionReason = "Another player has quit the game.";
            return false;
        }

        if (timer == 120) {
            naprint(exit, 120 - getVariableWidth(exit) / 2, 144);
        }

        if (timer >= 120) {
            if (key_hit(k.confirm)) {
                multiplayer = false;
                multiplayerLink->deactivate(true);

                sfx(SFX_MENUCANCEL);
                enableBlend(prevBld);
                playSongRandom(0);
                buildBG(3, 0, 27, 0, 1, true);
                changeScene(new TitleScene(), Transitions::FADE);
                return false;
            }

            cursorFloat += 6;
            if (cursorFloat >= 512)
                cursorFloat = 0;
            int offset = (sinLut(cursorFloat) * 2) >> 12;
            FIXED scale = float2fx((1.0 - ((float)0.1 * offset)));

            for (int i = 0; i < 2; i++) {
                sprite_unhide(cursorSprites[i], 0);
                sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 0);
                sprite_enable_affine(cursorSprites[i], i, true);
                sprite_set_size(cursorSprites[i], scale, i);

                int x = 240 / 2 -
                        ((getVariableWidth(exit) + 8) / 2 + offset + 4) *
                            ((i) ? -1 : 1) -
                        8;

                sprite_set_pos(cursorSprites[i], x, 144 - 5);
            }
        }

        showSprites(128);
        timer++;
    }

    clearTilemap(27);
    clearTilemap(26);
    buildBG(3, 0, 27, 0, 1, true);

    enableBlend(prevBld);

    for (auto& wordSprite : wordSprites)
        wordSprite.hide();

    for (auto& cursorSprite : cursorSprites)
        sprite_hide(cursorSprite);

    drawFrame(0);
    if (game->zoneTimer)
        frameSnow(0);

    showBackground(0);
    resetSmallText();
    clearText();
    setSmallTextArea(110, 3, 7, 9, 10);
    showText();

    if (!(game->won || game->lost)) {
        countdown();
    } else {
        endScreenSetup();
    }

    resumeSong();

    return true;
}
