#include "logging.h"
#include "multiplayerClasses.h"
#include "scene.hpp"
#include "sceneModes.hpp"
#include "sprites.h"
#ifdef GBA
#include "detectEmulators.h"
#else
#include "LinkWebRTC.hpp"
#endif
#include <cstring>
#include <utility>

bool lostConnection = true;

void PlayOptionScene::init() {
    loadSpriteTiles(620, &sprite49tiles_bin, 1, 1);
    for (int i = 0; i < 5; i++) {
        proSprites[i] = &obj_buffer[10 + i];
        sprite_hide(proSprites[i]);
    }

    OptionListScene::init();

    if (elementList.empty()) {
        onStart = true;
        if (getBoard(subMode, goal) != nullptr)
            onScore = true;
    }

    startSprite.setText("START");
}

bool PlayOptionScene::control() {
    MenuKeys k = savefile->settings.menuKeys;

    if ((onScore || getMode() == BlockEngine::ZEN) && key_is_down(k.special1) &&
        key_is_down(k.special2) && key_is_down(k.special3)) {
        resetScoreboard();
        refreshText = true;
        return false;
    }

    if (key_hit(k.cancel)) {
        sfx(SFX_MENUCANCEL);
        if (!(onScore || onStart) || options == 0) {
            previousElement = name();
            previousSelection = selection;

            if (!path.empty())
                path.pop_back();

            changeScene(previousScene(), Transitions::FADE);
            return true;
        } else if (onStart) {
            onStart = false;
        }

        if (onScore) {
            onScore = false;
            refreshText = true;
            movingDirection = -1;
            listStart = 0;
        }
    }

    if (!(onScore || onStart)) {
        if (key_hit(k.left) && !elementList.empty()) {
            auto it = elementList.begin();
            std::advance(it, selection);

            (*it)->action(-1);

            refreshOption = true;
        }

        if (key_hit(k.right) && !elementList.empty()) {
            auto it = elementList.begin();
            std::advance(it, selection);

            (*it)->action(1);

            refreshOption = true;
        }
    }

    int prev = listStart;

    if (key_hit(k.up)) {
        movingDirection = -1;

        bool sound = true;

        if (onScore && !elementList.empty()) {
            onScore = false;
            onStart = false;
            refreshText = true;
        } else if (onStart && !elementList.empty()) {
            onStart = false;
        } else {
            if (selection > 0)
                selection--;

            if (elementList.empty())
                sound = false;
        }

        if (selection == options - 1)
            listStart = max(0, selection - elementMax);
        else if (selection < listStart + 1) {
            listStart--;
            if (listStart < 0)
                listStart = 0;
        }

        if (sound)
            sfx(SFX_MENUMOVE);

        refreshText = true;
    }

    if (key_hit(k.down)) {
        if (!onScore) {
            movingDirection = 1;

            if (selection == options - 1) {
                if (getBoard(subMode, goal) != nullptr)
                    movingScore = true;
                else
                    onStart = true;
            } else
                selection++;

            if (selection == 0)
                listStart = 0;
            else if (selection > listStart + elementMax)
                listStart++;
            refreshText = true;
            sfx(SFX_MENUMOVE);
        }
    }

    if (!onScore && (key_is_down(k.up) || key_is_down(k.down))) {
        if (dasVer < maxDas) {
            dasVer++;
        } else {
            int dir = (key_is_down(k.up) ? -1 : 1);

            if (selection + dir >= 0 && selection + dir <= options - 1 &&
                arr++ > maxArr) {
                arr = 0;

                movingDirection = dir;

                selection += dir;

                if (dir < 0) {
                    if (selection == options - 1)
                        listStart = max(0, selection - elementMax);
                    else if (selection < listStart + 1) {
                        listStart--;
                        if (listStart < 0)
                            listStart = 0;
                    }
                } else {
                    if (selection == 0)
                        listStart = 0;
                    else if (selection > listStart + elementMax)
                        listStart++;
                }

                refreshText = true;
                sfx(SFX_MENUMOVE);
            }
        }
    } else {
        dasVer = 0;
    }

    if (!(onScore || onStart) &&
        (key_is_down(k.left) || key_is_down(k.right))) {
        if (dasHor < maxDas) {
            dasHor++;
        } else {
            int dir = (key_is_down(k.left) ? -1 : 1);
            if (arr++ > maxArr) {
                arr = 0;

                auto it = elementList.begin();
                std::advance(it, selection);

                (*it)->action(dir);

                refreshOption = true;
            }
        }
    } else {
        dasHor = 0;
    }

    if (prev != listStart) {
        moving = true;
    }

    if (key_hit(k.pause) || key_hit(k.confirm)) {
        if (!onScore && getBoard(subMode, goal) != nullptr) {
            movingScore = true;
            sfx(SFX_MENUCONFIRM);
        } else if (!onStart) {
            onStart = true;
            sfx(SFX_MENUCONFIRM);
        } else {
            start();
        }
    }

    return false;
}

constexpr int getDelayForBotDifficulty(int diff) {
    int delay = 0;

    float pps[] = {
        0.6, 1.5, 4, 10, 100,
    };

    delay = (int)((1.0 / pps[diff - 1]) * 60.0);

    return delay;
}

void PlayOptionScene::start() {
    bool training = false;
    int subMode = 0;
    int goal = 0;
    int bTypeHeight = 0;

    for (auto const& element : elementList) {
        std::string s = element->getLabel();
        int value = element->getValue();
        log(s);
        log(value);
        if (s == "Level") {
            level = value;
        } else if (s == "Type" || s == "Rules") {
            subMode = value;
        } else if (s == "Lines") {
            goal = value;
            mode = element->value;
        } else if (s == "Height") {
            bTypeHeight = value;
        } else if (s == "Finesse Training") {
            training = value;
        } else if (s == "Difficulty") {
            if (name() == "CPU Battle") {
                botThinkingSpeed = 12;
                botSleepDuration = getDelayForBotDifficulty(value);
                botStepMax = 10;
                mode = value;
            } else {
                goal = value;
                mode = element->value;
            }
        } else if (s == "Minutes") {
            goal = value;
            mode = element->value;
        }
    }

    BlockEngine::Options newOptions;

    newOptions.mode = getMode();

    if (newOptions.mode == BlockEngine::BLITZ)
        goal = 2 * 3600;

    if (newOptions.mode == BlockEngine::CLASSIC) {
        newOptions.rotationSystem = BlockEngine::NRS;
        newOptions.randomizer = BlockEngine::RANDOM;
    } else {
        newOptions.rotationSystem = savefile->settings.rotationSystem;

        if (savefile->settings.randomizer >= 0 &&
            savefile->settings.randomizer <= 2) {
            newOptions.randomizer = savefile->settings.randomizer;
        } else {
            newOptions.randomizer = BlockEngine::BAG_7;
        }
    }

    newOptions.goal = goal;
    newOptions.level = level;
    newOptions.tuning = getTuning();
    newOptions.bigMode = savefile->settings.big;
    newOptions.subMode = subMode;
    newOptions.trainingMode = training;
    newOptions.bTypeHeight = bTypeHeight;

    MenuKeys k = savefile->settings.menuKeys;

    proMode = (key_is_down(k.special1) || key_is_down(k.special2)) ^
              (savefile->settings.pro);

    startGame(newOptions, qran());

    gameLoop();
}

void PlayOptionScene::draw() {
    fallingBlocks();

    const int space = 1;
    const int startX = 3 * 8;
    const int endX = 30 - 4;

    int offset = (sinLut(cursorFloat) * 2) >> 12;
    auto scale = float2fx((1.0 - ((float)0.1 * offset)));

    int verticalOffset = 0;
    if (moving) {
        int temp =
            lerp((movingDirection) * (space + 1) * 8, 0, 64 * movingTimer);
        verticalOffset = lerp(temp, 0, 64 * movingTimer);
        setLayerScroll(2, 0, -verticalOffset);
    } else {
        setLayerScroll(2, 0, 0);
    }

    EntryBoard* board = getBoard(subMode, goal);

    if (!onScore) {
        int i = 0;       // drawing index, starts at listStart
        int counter = 0; // element index
        for (auto& option : elementList) {
            if (counter < listStart) {
                counter++;
                continue;
            }

            if (i > elementMax + 1) { // break if elements drawn reached max
                continue;
            }

            std::string opt = option->getCurrentOption();

            const int index = (i + listStart) % (elementMax + 2);

            if (refreshOption || refreshText) {
                if (counter == selection)
                    wordSprites[index]->setText(
                        option->getCursor(opt)); // draw option with text cursor
                else
                    wordSprites[index]->setText(
                        " " + opt); // draw option without text cursor
            }

            if (refreshText)
                labels[index]->setText(option->getLabel());

            const int x = (endX) * 8 - getVariableWidth(opt);
            const int y = startY + 8 * (space + 1) * i + verticalOffset;

            wordSprites[index]->setPriority(0);
            wordSprites[index]->show(
                x, y,
                15 - (counter != selection) -
                    (i == elementMax + 1 || (listStart && i == 0)));

            labels[index]->show(
                startX, y,
                15 - ((counter != selection) +
                      (i == elementMax + 1 || (listStart && i == 0))));

            if (counter == selection) { // draw sprite cursor
                currentOption = opt;
                currentElement = option->getLabel();
                for (int i = 0; i < 2; i++) {
                    sprite_unhide(cursorSprites[i], 0);
                    sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5,
                                    1);
                    sprite_enable_affine(cursorSprites[i], i, true);
                    sprite_set_size(cursorSprites[i], scale, i);

                    int x = startX - ((8) + offset + 4) - 4;

                    sprite_set_pos(cursorSprites[i], x, y - 5);
                }
            }

            i++;
            counter++;
        }

        for (int j = i; j < elementMax + 2; j++) {
            int index = (j + listStart) % (elementMax + 2);

            wordSprites[index]->hide();
            labels[index]->hide();
        }

        for (int i = elementMax + 2; i < 9; i++)
            wordSprites[i]->hide();

        for (auto& proSprite : proSprites)
            sprite_hide(proSprite);
    } else {
        if (board != nullptr) {
            for (int i = 0; i < 5; i++) {
                const int y = startY + 8 * (1) * i + verticalOffset;

                char buff[12];
                posprintf(buff, "%d. %s", i + 1, board->entries[i].name);

                labels[i]->setText(buff);
                labels[i]->show(startX, y, 15);

                std::string str;
                if (getIfGrade() && board->entries[i].value > 0) {
                    if (name() == "Master") {
                        str += GameInfo::masterGrades[board->entries[i].grade];
                    } else if (name() == "Death") {
                        str += GameInfo::deathGrades[board->entries[i].grade];
                    }
                    str += " ";
                }

                if (!getIfTime()) {
                    str += std::to_string(board->entries[i].value);
                } else
                    str += timeToString(board->entries[i].value, false);

                const int x = (endX) * 8 - getVariableWidth(str);
                wordSprites[i]->setText(str);
                wordSprites[i]->show(x, y, 15);

                if (board->entries[i].pro) {
                    sprite_unhide(proSprites[i], 0);

                    sprite_set_attr(proSprites[i], ShapeSquare, 0, 620, 15, 0);
                    sprite_set_pos(proSprites[i], endX * 8 + 4, y);
                } else {
                    sprite_hide(proSprites[i]);
                }
            }
        }

        for (int j = 5; j < elementMax + 2; j++)
            labels[j]->hide();

        for (int j = 6; j < elementMax + 2; j++)
            wordSprites[j]->hide();

        int i = 0;
        for (auto const& option : elementList) {
            if (option->getLabel() == "Level" &&
                !(name() == "Classic" && subMode))
                continue;

            wordSprites[6 + i]->setText(option->getCurrentOption());
            wordSprites[6 + i]->show(30 * 8 - 4 - wordSprites[6 + i]->width,
                                     4 + i * 8, 14);

            i++;
        }
    }

    if (onScore || board == nullptr) {
        startSprite.show((240 - 43) / 2, 132, 15);

        if (onScore || onStart) {
            for (int i = 0; i < 2; i++) {
                sprite_unhide(cursorSprites[i], 0);
                sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
                sprite_enable_affine(cursorSprites[i], i, true);
                sprite_set_size(cursorSprites[i], scale, i);

                int x =
                    240 / 2 - ((43 + 8) / 2 + offset + 4) * ((i) ? -1 : 1) - 10;

                sprite_set_pos(cursorSprites[i], x, 132 - 5);
            }
        }

        for (auto& scrollSideSprite : scrollSideSprites) {
            sprite_hide(scrollSideSprite);
        }
    } else {
        startSprite.hide();
    }

    if (listStart > 0) {
        sprite_unhide(arrowSprites[0], 0);
        sprite_set_attr(arrowSprites[0], ShapeWide, 0, 1, 14, 0);
        sprite_set_pos(arrowSprites[0], 240 / 2 - 8, startY - 11 - 16);
    } else {
        sprite_hide(arrowSprites[0]);
    }

    if (!onScore && getBoard(subMode, goal) != nullptr) {
        sprite_unhide(arrowSprites[1], 0);
        sprite_set_attr(arrowSprites[1], ShapeWide, 0, 1, 14, 0);
        sprite_enable_flip(arrowSprites[1], false, true);
        sprite_set_pos(arrowSprites[1], 240 / 2 - 8,
                       (elementMax + 2) * (1 + space) * 8 + startY - 48);
    } else {
        sprite_hide(arrowSprites[1]);
    }

    if (refreshText || refreshOption) {
        setOptions();
        std::string next =
            getDescription(name(), currentElement, currentOption);

        if (next.empty() || next != scrollText) {
            scrollText = next;
            clearSpriteTiles(672, 144, 1);
            if (!scrollText.empty()) {
                scrollTextLength = getVariableWidth(scrollText);

                const int width =
                    (savefile->settings.aspectRatio == 0) ? 240 : 214;

                endDelay = 0;
                scrollOffset = -max((width - scrollTextLength), 0) / 2;
                scrollDelay = 0;
                scrollingText[0]->setTextSlow(scrollText);

#ifdef TE
                for (int i = 0; i < 2; i++) {
                    sprite_unhide(scrollSideSprites[i], ATTR0_AFF);
                    sprite_set_pos(scrollSideSprites[i],
                                   240 / 2 +
                                       ((i * 2) - 1) *
                                           (min(scrollTextLength, width) / 2) -
                                       16 * (i == 0),
                                   152 - 4);
                }
#endif
            } else {
                scrollingText[0]->setTextSlow(scrollText);

                for (auto& scrollSideSprite : scrollSideSprites) {
                    sprite_hide(scrollSideSprite);
                }
            }
        }
    }

    showSprites(128);

    refreshText = false;
    refreshOption = false;
}

void PlayOptionScene::update() {
    if (movingScore > 0) {
        if (++movingScoreTimer >= movingScoreTimerMax) {
            if (++listStart >= options - 1) {
                movingScore = 0;
                onScore = true;
                onStart = true;
                clearSpriteTiles(672, 144, 1);
                scrollingText[0]->setText("");
                scrollText = "";
                selection = elementList.size() - 1;
            }
        }
    }

    OptionListScene::update();
}

void PlayOptionScene::setOptions() {
    for (auto const& element : elementList) {
        std::string s = element->getLabel();
        int value = element->value;
        if (s == "Level") {
            level = value;
        } else if (s == "Type" || s == "Rules") {
            subMode = value;
        } else if (s == "Lines") {
            goal = value;
        } else if (s == "Minutes") {
            goal = value;
        } else if (s == "Difficulty") {
            goal = value;
        }
    }
}

#ifndef MULTIBOOT

void ClassicScene::update() {
    if (!subMode) {
        if (elementList.back()->getLabel() == "Height") {

            Element* e = elementList.back();
            elementList.pop_back();

            delete e;
            options--;
        }
    } else {
        if (elementList.back()->getLabel() != "Height") {
            elementList.push_back(
                new IntSelectorElement("Height", {0, 1, 2, 3, 4, 5}));
            refreshText = true;
            options++;
        }
    }

    PlayOptionScene::update();
}

#endif

int lastState = 0;
void MultBattleScene::init() {
    PlayOptionScene::init();

#ifdef GBA

    key_poll();
    // Secret feature: hold L on real hardware to force cable mode
    if (!inaccurateEmulator && (key_held(KEY_L) || logInitMgba()))
        // mGBA only supports cable, so set it to speed up initial multiplayer
        // sync
        multiplayerLink->universal->setProtocol(LinkUniversal::CABLE);
    else if (inaccurateEmulator)
        // gpSP only does wireless
        multiplayerLink->universal->setProtocol(LinkUniversal::WIRELESS_AUTO);
    else
        // This undoes the hold L override if exiting and returning to this
        // screen
        multiplayerLink->universal->setProtocol(LinkUniversal::AUTODETECT);

    if (!inaccurateEmulator) {
        rumbleHandler = interrupt_get_handler(INTR_SERIAL);
        interrupt_set_handler(INTR_SERIAL, LINK_UNIVERSAL_ISR_SERIAL);
    }

#endif

    multiplayerLink->active = true;

    lastState = 0;

    clearSpriteTiles(672, 144, 1);
    resetSmallText();
}

void MultBattleScene::draw() {
    fallingBlocks();

    handleConnectionDisplay();

    showSprites(128);
}

void MultBattleScene::handleConnectionDisplay() {
    int state;
    if (multiplayerLink->players[multiplayerLink->universal->currentPlayerId()]
            .ready)
        state = 1;
    else if (multiplayerLink->stateMachine
                 .IsInState<MultiplayerStates::PlayersAwaitReady>())
        state = 2;
    else
        state = 3;

    if (multiplayerLink->universal->isConnected())
        showReadyPlayers();
    else
        hideReadyPlayers();

    if (lastState == state)
        return;

    clearText();
    lastState = state;
    switch (state) {
    case 1:
    case 2:
        naprint("Connected!", 10 * 8, 8 * 8);
        if (state == 1)
            naprint("                    ", 6 * 8, 15 * 8);
        else
            naprint("Press A to ready up!", 6 * 8, 15 * 8);
        naprint("Waiting for all players...", 4 * 8, 10 * 8);
        break;
    default:
        naprint("Waiting for connection...", 5 * 8, 9 * 8);
        break;
    }
}

bool MultBattleScene::control() {
    if (multiplayerStart) {
        multiplayerStart = false;
        changeScene(new GameScene(), Transitions::FADE);
        return true;
    }
    MenuKeys k = savefile->settings.menuKeys;

    if (key_hit(k.cancel)) {
        sfx(SFX_MENUCANCEL);
        if (!onScore || options == 0) {
            previousElement = name();
            previousSelection = selection;

            if (!path.empty())
                path.pop_back();

            changeScene(previousScene(), Transitions::FADE);

            multiplayerLink->deactivate(false);

            return true;
        } else {
            onScore = false;
            refreshText = true;
            movingDirection = -1;
            listStart = 0;
        }

    } else if ((multiplayerLink->stateMachine
                    .IsInState<MultiplayerStates::PlayersAwaitReady>()) &&
               !multiplayerLink
                    ->players[multiplayerLink->universal->currentPlayerId()]
                    .ready &&
               (key_hit(k.confirm) || key_hit(k.pause))) {
        sfx(SFX_MENUCONFIRM);
        multiplayerLink->readyUp();
    }

    return false;
}

void MultBattleScene::update() {
    PlayOptionScene::update();

    if (!multiplayerLink->active)
        return;

    if (multiplayerLink->sync()) {
        if (lostConnection) {
            if (multiplayerLink->active)
                sfx(SFX_SECRET);
            lostConnection = false;
        }
    } else {
        if (!lostConnection) {
            refreshText = true;
            clearText();
            lastState = 0;
            // TODO: Should go into commitPlayerCount
        }
        lostConnection = true;
    }
}

void MultBattleScene::deinit() {
    PlayOptionScene::deinit();
    playAgain = false;
#ifdef GBA
    multiplayerLink->deactivate(false);
    if (!inaccurateEmulator) {
        if (rumbleHandler)
            interrupt_set_handler(INTR_SERIAL, rumbleHandler);
    }
#endif
}

#ifndef GBA
MultBattleScene::MultBattleScene() { this->matchMake = true; }

MultBattleScene::MultBattleScene(std::string room) { roomId = std::move(room); }
#endif

void PlayOptionScene::resetScoreboard() {
    vsync();

    resetSmallText();
    clearText();

    clearSprites(128);
    showSprites(128);

    int timer = 0;

    int s = 1;

    naprint("Are you sure you", 6 * 8, 6 * 8);
    naprint("want to reset the", 6 * 8, 8 * 8);
    naprint("current scoreboard?", 6 * 8, 10 * 8);

    bool reset = false;

    MenuKeys k = savefile->settings.menuKeys;

    while (closed()) {
        vsync();
        key_poll();

        fallingBlocks();

        if (timer < 200) {
            timer++;
        } else {
            aprint(" YES      NO ", 8, 15);

            int cursorX = 0;
            int length = 0;
            if (s == 0) {
                cursorX = 11 * 8 - 2;
                length = 3 * 8;
            } else {
                cursorX = 19 * 8 + 2;
                length = 2 * 8;
            }

            int offset = (sinLut(cursorFloat) * 2) >> 12;
            auto scale = float2fx((1.0 - ((float)0.1 * offset)));

            for (int i = 0; i < 2; i++) {
                sprite_unhide(cursorSprites[i], 0);
                sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
                sprite_enable_affine(cursorSprites[i], i, true);
                sprite_set_size(cursorSprites[i], scale, i);

                int x = cursorX -
                        ((length + 8) / 2 + offset + 4) * ((i) ? -1 : 1) - 10;

                sprite_set_pos(cursorSprites[i], x, 15 * 8 - 5);
            }

            if (key_hit(k.confirm)) {
                clearText();
                clearSprites(128);
                sfx(SFX_MENUCONFIRM);
                if (s == 0) {
                    reset = true;
                    break;
                } else {
                    break;
                }
            }

            if (key_hit(k.cancel)) {
                clearText();
                sfx(SFX_MENUCANCEL);
                break;
            }

            if (key_hit(k.left)) {
                if (s > 0) {
                    s--;
                    sfx(SFX_MENUMOVE);
                }
            }

            if (key_hit(k.right)) {
                if (s < 1) {
                    s++;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        showSprites(128);

        cursorFloat += 6;
        if (cursorFloat >= 512)
            cursorFloat = 0;
    }

    if (reset) {
        if (getMode() != BlockEngine::ZEN) {
            EntryBoard* board = getBoard(subMode, goal);

            memset32_fast(board, 0, sizeof(EntryBoard) / 4);
        } else {
            savefile->boards.zen = 0;
        }

        saveSavefile();
    }

    showPath();
}

void MatchMakingScene::init() {
    clearSprites(128);
    showSprites(128);

    clearSpriteTiles(256, 12 * 2, 1);
    for (int i = 0; i < 2; i++)
        wordSprites[i].setup(i, 64 + i * 3, 256 + i * 12, false);

    clearTilemap(29);
    naprint("Who do you want to play with?", 40, 56);

    wordSprites[0].setText("FRIENDS");
    wordSprites[0].show(48, 104, 14);

    wordSprites[1].setText("RANDOMS");
    wordSprites[1].show(120, 104, 14);

    // backgroundGrid
    setTiles(26, 0, 32 * 32,
             tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));

    setTiles(27, 0, 32 * 32, tileBuild(34, false, false, 0));

    lengths[0] = wordSprites[0].width;
    lengths[1] = wordSprites[1].width;

    pos[0] = 48 + lengths[0] / 2;
    pos[1] = 120 + lengths[1] / 2;

    loadSpriteTiles(16 * 7, blockSprite, 1, 1);
    for (int i = 0; i < 2; i++) {
        cursorSprites[i] = &obj_buffer[1 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_hide(cursorSprites[i]);
    }

    enableBlend((0b101110 << 8) + (1 << 6) + (1 << 3));
}

bool MatchMakingScene::control() {
    MenuKeys k = savefile->settings.menuKeys;

    if (key_hit(k.left) || key_hit(k.right)) {
        playRandoms = !playRandoms;
        sfx(SFX_MENUMOVE);
    }

    if (key_hit(k.confirm)) {
        sfx(SFX_MENUCONFIRM);
        if (!playRandoms) {
            clearText();
            clearSmallText();
            hideSprites = true;
#ifdef GBA
            changeScene(new MultBattleScene(), Transitions::FADE);
#else
            roomId = nameInput(-1);
            changeScene(new MultBattleScene(roomId), Transitions::FADE);
#endif
        } else {
#ifdef GBA
            changeScene(new MultBattleScene(), Transitions::FADE);
#else
            roomId = "";
            changeScene(new MultBattleScene(), Transitions::FADE);
#endif
        }
    } else if (key_hit(k.cancel)) {
        sfx(SFX_MENUCANCEL);
        changeScene(new MainMenuScene(), Transitions::FADE);
    }

    return false;
}

void MatchMakingScene::draw() {
    fallingBlocks();

    int offset = (sinLut(cursorFloat) * 2) >> 12;
    FIXED scale = float2fx((1.0 - ((float)0.1 * offset)));

    const int y = 104;

    for (int i = 0; i < 2; i++) {
        sprite_unhide(cursorSprites[i], 0);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_set_size(cursorSprites[i], scale, i);

        int x = pos[playRandoms] -
                ((lengths[playRandoms] + 8) / 2 + offset + 4) * ((i) ? -1 : 1) -
                8;

        sprite_set_pos(cursorSprites[i], x, y - 5);
    }

    if (!hideSprites) {
        wordSprites[0].show(48, 104, 14 + (playRandoms == 0));
        wordSprites[1].show(120, 104, 14 + (playRandoms == 1));
    } else {
        wordSprites[0].hide();
        wordSprites[1].hide();
    }

    showSprites(128);
}

void MatchMakingScene::update() {
    canDraw = true;
    key_poll();

    cursorFloat += 6;
    if (cursorFloat >= 512)
        cursorFloat = 0;

    control();
}

void MatchMakingScene::deinit() {
    clearText();
    clearSmallText();
    resetSmallText();
}
