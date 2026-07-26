#include "sceneMultiplayerResults.hpp"
#include "def.h"
#include "sceneModes.hpp"
#include "sprites.h"
#include "text.h"

void MultiplayerResultsScene::buildDisplayLines() {
    displayLines.clear();

    if (multiplayerLink->targetWins > 0) {
        std::string winner;
        if (multiplayerLink->wins >= multiplayerLink->targetWins) {
            winner = "You win the match!";
        } else {
            // Someone else reached target - find who has the most wins
            winner = "Match complete!";
        }
        displayLines.push_back(winner);
        displayLines.push_back("");
    }

#ifndef GBA
    auto playerNames = multiplayerLink->getPlayerNames();
    u8 localId = multiplayerLink->universal->currentPlayerId();
    std::string localName = playerNames[localId];
    if (localName.empty())
        localName = "You";
#endif

    int gameNum = 1;
    for (const auto& result : multiplayerLink->matchHistory) {
        std::string line;
        line = "Game " + std::to_string(gameNum) + ": ";

        if (result.playerCount <= 2) {
            // 2P: show who won
            if (result.won) {
                line += "You won!";
            } else {
                line += "You lost";
            }
        } else {
            // >2P: show placement
            int place = result.placement + 1;
            line += std::to_string(place);
            // Ordinal suffix
            if (place == 1)
                line += "st";
            else if (place == 2)
                line += "nd";
            else if (place == 3)
                line += "rd";
            else
                line += "th";
            line += " place";
        }
        displayLines.push_back(line);
        gameNum++;
    }

    displayLines.push_back("");
    displayLines.push_back("Wins: " + std::to_string(multiplayerLink->wins) +
                           "  Losses: " + std::to_string(multiplayerLink->losses));
}

void MultiplayerResultsScene::init() {
    reset();
    resetSmallText();
    clearText();
    clearSprites(128);

    buildDisplayLines();

    // backgroundGrid
    setTiles(26, 0, 32 * 32,
             tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));
    setTiles(27, 0, 32 * 32, tileBuild(34, false, false, 0));

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i] = new WordSprite(i, 64 + i * 3, 256 + i * 12);

    loadSpriteTiles(16 * 7, blockSprite, 1, 1);
    for (int i = 0; i < 2; i++) {
        cursorSprites[i] = &obj_buffer[1 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_hide(cursorSprites[i]);
    }

    enableBlend((0b101111 << 8) + (1 << 6) + (1 << 3));

    path.clear();
    path.emplace_back("Play");
    path.emplace_back("Multi Battle");
    path.emplace_back("Results");

    listStart = 0;
    selection = 0;
    renderText();
}

void MultiplayerResultsScene::renderText() {
    clearText();

    naprint("MATCH RESULTS", 120 - getVariableWidth("MATCH RESULTS") / 2,
            1 * 8);

    int visibleCount = 0;
    for (int i = listStart;
         i < (int)displayLines.size() && visibleCount < maxVisible; i++) {
        if (displayLines[i].empty())
            continue;
        naprint(displayLines[i],
                120 - getVariableWidth(displayLines[i]) / 2,
                (3 + visibleCount) * 8);
        visibleCount++;
    }

    const int buttonY = 3 + maxVisible + 1;
    naprint("Play Again", 120 - getVariableWidth("Play Again") / 2,
            buttonY * 8);
    naprint("Main Menu", 120 - getVariableWidth("Main Menu") / 2,
            (buttonY + 2) * 8);
}

void MultiplayerResultsScene::draw() {
    fallingBlocks();
    toggleBG(3, true);
    showSprites(128);
}

void MultiplayerResultsScene::update() {
    canDraw = true;
    key_poll();

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

        int buttonY = 3 + maxVisible + 1 + selection * 2;
        int x = 240 / 2 -
                ((getVariableWidth(selection == 0 ? "Play Again"
                                                  : "Main Menu") +
                  8) /
                     2 +
                 offset + 4) *
                    (i ? -1 : 1) -
                8;

        sprite_set_pos(cursorSprites[i], x, buttonY * 8 - 5);
    }

    control();
}

bool MultiplayerResultsScene::control() {
    MenuKeys k = savefile->settings.menuKeys;

    if (key_hit(k.up)) {
        if (selection > 0) {
            selection--;
            sfx(SFX_MENUMOVE);
        }
    }
    if (key_hit(k.down)) {
        if (selection < optionCount - 1) {
            selection++;
            sfx(SFX_MENUMOVE);
        }
    }

    if (key_hit(k.confirm) || key_hit(k.pause)) {
        if (selection == 0) {
            // Play Again
            sfx(SFX_MENUCONFIRM);
            multiplayerLink->resetSession();
            multiplayerLink->playAgain = true;
#ifndef GBA
            changeScene([]() { return new MultBattleScene(roomId); },
                        Transitions::FADE);
#else
            changeScene([]() { return new MultBattleScene(); },
                        Transitions::FADE);
#endif
            return true;
        } else if (selection == 1) {
            // Main Menu
            sfx(SFX_MENUCANCEL);
            multiplayerLink->deactivate(true);
            multiplayer = false;
            path.clear();
#ifndef MULTIBOOT
            changeScene([]() { return new MainMenuScene(); },
                        Transitions::FADE);
#else
            changeScene([]() { return new MultiplayerProtocolScene(); },
                        Transitions::FADE);
#endif
            return true;
        }
    }

    if (key_hit(k.cancel)) {
        sfx(SFX_MENUCANCEL);
        multiplayerLink->deactivate(true);
        multiplayer = false;
        path.clear();
#ifndef MULTIBOOT
        changeScene([]() { return new MainMenuScene(); },
                    Transitions::FADE);
#else
        changeScene([]() { return new MultiplayerProtocolScene(); },
                    Transitions::FADE);
#endif
        return true;
    }

    return false;
}

void MultiplayerResultsScene::deinit() {
    clearSprites(128);
    showSprites(128);
    clearText();

    for (auto& wordSprite : wordSprites)
        delete wordSprite;

    for (auto& cursorSprite : cursorSprites)
        sprite_hide(cursorSprite);
}
