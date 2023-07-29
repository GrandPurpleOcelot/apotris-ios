#include "def.h"
#include "rumble.h"
#include "scene.hpp"
#include "tetromino.hpp"
// #include "soundbank.h"
#include "tetrisEngine.h"
#include "logging.h"
#include "text.h"
#include <string>
#include <map>
#include <cstring>
#include "posprintf.h"

using namespace Tetris;

void endAnimation();
void showScore();
void showStats(bool, std::string, std::string);
int onRecord();
std::string nameInput(int);
void showModeText();

void saveReplay();

int mode = 0;
bool showingStats = false;

bool saveExists = false;
Tetris::Game* quickSave;
Tetris::Bot *testBot;

int igt = 0;

const std::string modeStrings[11] = {
    "Marathon",
    "Sprint",
    "Dig",
    "Battle",
    "Ultra",
    "Blitz",
    "Combo",
    "Survival",
    "Classic",
    "Master",
    "Training",
};

const std::string modeOptionStrings[11][4] = {
    {"150","200","300","Endless"},
    {"20","40","100"},
    {"10","20","100"},
    {""},
    {"3","5","10"},
    {""},
    {""},
    {"EASY","MEDIUM","HARD"},
    {"",""},
    {"Normal","Classic"},
    {""},
};

void songListMenu() {
    int startX = 3;
    int endX = 24;

    int startY = 3;

    int selection = 0;
    int options = 7;

    stopSong();
    playSong(0, 0);
    bool previous = savefile->settings.cycleSongs;
    savefile->settings.cycleSongs = false;

    while (closed()) {
        vsync();
        // VBlankIntrWait();

        key_poll();

        if (key_hit(KEY_START)) {
            sfx(SFX_MENUCONFIRM);
            if (selection != options - 1) {
                selection = options - 1;
            } else {
                break;
            }
        }

        if (key_hit(KEY_B)) {
            sfx(SFX_MENUCANCEL);
            if (selection == options - 1)
                selection = 0;
            else
                break;
        }

        if (key_hit(KEY_UP)) {
            if (selection > 0)
                selection--;
            else
                selection = options - 1;
            sfx(SFX_MENUMOVE);

            stopSong();
            // mmStop();
            if (selection < 2)
                playSong(0, selection);
            else if (selection < 6)
                playSong(1, selection - 2);
        }

        if (key_hit(KEY_DOWN)) {
            if (selection < options - 1)
                selection++;
            else
                selection = 0;
            sfx(SFX_MENUMOVE);
            if (selection < 2)
                playSong(0, selection);
            else if (selection < 6)
                playSong(1, selection - 2);
        }

        if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT) || key_hit(KEY_A)) {
            sfx(SFX_MENUCONFIRM);
            if (selection == options - 1)
                break;
            else {
                savefile->settings.songList[selection] = !savefile->settings.songList[selection];
            }
        }

        aprint(" DONE ", 12, 17);

        aprint("Menu:", startX, startY);
        aprint("Track 1", startX, startY + 2);
        aprint("Track 2", startX, startY + 3);

        aprint("Game:", startX, startY + 6);
        aprint("Track 1", startX, startY + 8);
        aprint("Track 2", startX, startY + 9);
        aprint("Track 3", startX, startY + 10);
        aprint("Track 4", startX, startY + 11);

        for (int i = 0; i < 2; i++)
            aprint("   ", endX - 1, startY + 2 + i);

        for (int i = 0; i < 6; i++)
            aprint("   ", endX - 1, startY + 7 + i);

        if (savefile->settings.songList[0])
            aprint("x", endX, startY + 2);
        if (savefile->settings.songList[1])
            aprint("x", endX, startY + 3);

        if (savefile->settings.songList[2])
            aprint("x", endX, startY + 8);
        if (savefile->settings.songList[3])
            aprint("x", endX, startY + 9);
        if (savefile->settings.songList[4])
            aprint("x", endX, startY + 10);
        if (savefile->settings.songList[5])
            aprint("x", endX, startY + 11);

        if (selection == options - 1) {
            aprint("[", 12, 17);
            aprint("]", 17, 17);
        } else {
            aprint("[", endX - 1, startY + 2 + selection + (selection > 1) * 4);
            aprint("]", endX + 1, startY + 2 + selection + (selection > 1) * 4);
        }

        showSprites(128);
    }

    stopSong();
    playSongRandom(0);

    savefile->settings.cycleSongs = previous;
}

int endScreen() {
    clearSmallText();
    setSmallTextArea(110, 1, 1, 10, 20);

    stopSong();

    if(!replaying)
        saveReplay();
    else
        replaying = false;

    int selection = 0;

    //calculate pps
    FIXED t = gameSeconds * float2fx(0.0167f);
    FIXED pps =  fxdiv(int2fx(game->pieceCounter),(t));

    std::string ppsStr = std::to_string(fx2int(pps)) + ".";

    int fractional = pps & 0xff;
    for(int i = 0; i < 2; i++){
        fractional *= 10;
        ppsStr += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

    std::string totalTime = timeToString(gameSeconds);

    endAnimation();

    int prevBld = blendInfo;
    enableBlend((1 << 6) + (0b11101 << 9) + (1));

    setTiles(25,0,32*32,tileBuild(34, false, false, 0));

    drawFrame(1);

    if(savefile->settings.announcer){
        if (game->gameMode == ULTRA || game->gameMode == BLITZ) {
            if (game->won == 1)
                sfx(SFX_TIME);
            else if (game->lost == 1)
                sfx(SFX_GAMEOVER);
        }else if (game->gameMode != BATTLE) {
            if (game->gameMode == MASTER && game->won == 1 && game->level < 500 && game->timer >= 25200)
                sfx(SFX_TIME);
            else if (game->won == 1)
                sfx(SFX_CLEAR);
            else if (game->lost == 1)
                sfx(SFX_GAMEOVER);
        } else {
            if (game->won == 1)
                sfx(SFX_YOUWIN);
            else if (game->lost == 1)
                sfx(SFX_YOULOSE);
        }
    }

    igt = game->inGameTimer;

    resetZonePalette();

    playSongRandom(0);

    showScore();
    int record = onRecord();

    if (multiplayer) {
        enemyHeight = 0;
        progressBar(0);
    }

    savefile->stats.gamesCompleted++;
    if(game->lost)
        savefile->stats.gamesLost++;

    showStats(showingStats, totalTime, ppsStr);

    while (closed()) {
        handleMultiplayer(1);
        vsync();
        key_poll();

        showScore();

        if (record != -1 && game->gameMode != BATTLE && (game->won || game->gameMode == MARATHON || game->gameMode >= ULTRA)){
            std::string str;
            switch(record){
                case 0: str = "1st"; break;
                case 1: str = "2nd"; break;
                case 2: str = "3rd"; break;
                case 3: str = "4th"; break;
                case 4: str = "5th"; break;
            }
            aprint(str+" Place", 11, 5);
        }

        showModeText();

        aprint("Play", 12, 11);
        aprint("Again", 14, 12);

        aprint("Change", 12, 14);
        aprint("Options", 14, 15);

        aprint("Main", 12, 17);
        aprint("Menu", 14, 18);

        if (selection == 0) {
            aprint(">", 10, 11);
            aprint(" ", 10, 14);
            aprint(" ", 10, 17);
        } else if (selection == 1) {
            aprint(" ", 10, 11);
            aprint(">", 10, 14);
            aprint(" ", 10, 17);
        } else if (selection == 2) {
            aprint(" ", 10, 11);
            aprint(" ", 10, 14);
            aprint(">", 10, 17);
        }

        if (playAgain && multiplayer) {
            startMultiplayerGame(nextSeed);

            stopSong();
            playSongRandom(1);

            floatingList.clear();
            placeEffectList.clear();

            drawFrame(0);
            clearText();

            clearSprites(128);
            showHold();
            showQueue();
            showSprites(128);
            clearTilemap(25);
            // countdown();
            // updateText();
            break;
        }

        if (key_hit(KEY_A) || key_hit(KEY_START)) {
            if (selection == 0) {
                shake = 0;

                if (!multiplayer) {
                    sfx(SFX_MENUCONFIRM);
                    startGame();
                    changeScene(new GameScene());
                    break;
                } else {
                    if (connected == 1) {
                        sfx(SFX_MENUCONFIRM);
                        multiplayerStartTimer = 5;
                        nextSeed = (u16)qran();
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

            } else if (selection == 1){
                sfx(SFX_MENUCANCEL);
                enableBlend(prevBld);
                // REG_BLDCNT = prevBld;
                goToOptions = true;
                nextSeed = 0;
                return 1;
            } else if (selection == 2){
                sfx(SFX_MENUCANCEL);
                enableBlend(prevBld);
                goalSelection = 0;
                level = 1;
                changeScene(new MainMenuScene, Transitions::FADE);
                return 1;
            }
        }

        if (key_hit(KEY_UP)) {
            if (selection == 0)
                selection = 2;
            else
                selection--;
            sfx(SFX_MENUMOVE);
        }

        if (key_hit(KEY_DOWN) || key_hit(KEY_SELECT)) {
            if (selection == 2)
                selection = 0;
            else
                selection++;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_L)){
            sfx(SFX_MENUCONFIRM);
            showingStats = !showingStats;
            clearSmallText();
            showStats(showingStats, totalTime, ppsStr);
        }

        showSprites(128);
    }

    enableBlend(prevBld);
    return 0;
}

void endAnimation() {
    clearText();
    clearSprites(128);
    showHold();
    showQueue();
    showSprites(128);

    setLayerScroll(0, 0, 0);
    setLayerScroll(1, 0, 0);

    //TODO::
    #ifdef GBA
    rumble_set_state(RumbleState(rumble_stop));
    #endif

    sfx(SFX_END);

    for (int i = 0; i < 40; i++) {
        vsync();
        drawGrid();
        showClearText();
        // setLayerScroll(0, 0, -4 * i);
        // setLayerScroll(1, 0, -4 * i);

        if (i % 2 != 0)
            continue;

        showBackground(i/2);
        for (int j = 0; j < 10; j++) {
            setTile(25, j + 10, i/2,0);
            // setTile(25, j + 10, 20-(i/2),0);
            // *dest++ = 0;
        }

    }

    int counter = 0;
    while (!floatingList.empty()) {
        vsync();
        showClearText();

        if(++counter > 120)
            break;
    }
    clearText();

    setLayerScroll(0, 0, 0);
    setLayerScroll(1, 0, 0);
}

void showScore(){
    if (game->gameMode == BATTLE) {
        if (game->lost)
            aprint("YOU LOSE", 11, 3);
        else
            aprint("YOU WIN!", 11, 3);

        aprint("Lines Sent", 10, 5);
        aprintf(game->linesSent, 14, 7);

    } else if (game->gameMode == MASTER){
        int grade = game->grade + game->coolCount + (int) game->creditGrade;
        if(grade > 32)
            grade = 32;

        std::string gradeText = GameInfo::masterGrades[grade];

        if(gradeText[0] == ' ')
            gradeText.erase(0,1);

        gradeText = "Grade: " + gradeText;

        if(game-> won && game->level < 500 && game->timer >= 25200) {
            aprint("TIME!", 13, 3);
        }else if(game->won && game->level == 999){
            aprint("CLEAR!", 12, 3);
        }else{
            aprint("GAME OVER", 11, 3);
        }

        aprint(gradeText, 15 - ((int)gradeText.size()/2),7);

    } else if (game->gameMode == MARATHON || game->lost || game->gameMode >= ULTRA || (game->gameMode == DIG && subMode == 1)) {
        std::string score;

        if(game->gameMode == DIG)
            score = std::to_string(game->pieceCounter);
        else if (game->gameMode != 7)
            score = std::to_string(game->score);
        else
            score = std::to_string(game->linesCleared);

        if (game->lost)
            aprint("GAME OVER", 11, 3);
        else {
            if (game->gameMode != ULTRA && game->gameMode != BLITZ)
                aprint("CLEAR!", 12, 3);
            else
                aprint("TIME!", 13, 3);
        }

        if (game->gameMode == MARATHON || game->gameMode == ULTRA || game->gameMode == BLITZ || game->gameMode == COMBO || game->gameMode == DIG || game->gameMode == CLASSIC)
            aprint(score, 15 - ((int)score.size() / 2), 7);
        else if (game->gameMode == SURVIVAL)
            aprint(timeToString(gameSeconds), 11, 7);
    } else {
        aprint("CLEAR!", 12, 3);

        aprint(timeToString(gameSeconds), 11, 7);
    }
}

void showStats(bool moreStats, std::string time, std::string pps) {
    if(!moreStats){
        aprints("Press L", 0, 0, 2);
        aprints("for Stats", 0, 7, 2);
        return;
    }

    int counter = 0;

    int g = game->gameMode;

    aprints("Time: " + time,0,7*counter++,2);
    if(!proMode)
        aprints("IGT: " + timeToString(igt),0,7*counter++,2);

    if(g == MARATHON || g == BLITZ || g == CLASSIC){
        if(g != BLITZ)
            aprints("Start Level: " + std::to_string(game->initialLevel),0,7*counter++,2);

        aprints("Final Level: " + std::to_string(game->level),0,7*counter++,2);
        aprints("Score: " + std::to_string(game->score),0,7*counter++,2);
    }else if (g == ULTRA){
        aprints("Score: " + std::to_string(game->score),0,7*counter++,2);
    }else if (g == MASTER){
        aprints("Level: " + std::to_string(game->level),0,7*counter++,2);
    }

    aprints("Lines: " + std::to_string(game->linesCleared),0,7*counter++,2);
    aprints("Pieces: " + std::to_string(game->pieceCounter), 0, 7*counter++, 2);

    aprints("PPS: " + pps, 0, 7*counter++, 2);

    if(game->rotationSystem == SRS)
        aprints("Finesse: " + std::to_string(game->finesseFaults), 0, 7*counter++, 2);

    aprints("Singles: " + std::to_string(game->statTracker.clears[0]), 0, 7*counter++, 2);
    aprints("Doubles: " + std::to_string(game->statTracker.clears[1]), 0, 7*counter++, 2);
    aprints("Triples: " + std::to_string(game->statTracker.clears[2]), 0, 7*counter++, 2);
    aprints("Quads: " + std::to_string(game->statTracker.clears[3]), 0, 7*counter++, 2);

    if(game->gameMode != CLASSIC){
        aprints("T-Spins: " + std::to_string(game->statTracker.tspins), 0, 7*counter++, 2);
        aprints("Perfect Clears: " + std::to_string(game->statTracker.perfectClears), 0, 7*counter++, 2);
        aprints("Max Streak: " + std::to_string(game->statTracker.maxStreak), 0, 7*counter++, 2);
        aprints("Max Combo: " + std::to_string(game->statTracker.maxCombo), 0, 7*counter++, 2);
        aprints("Times Held: " + std::to_string(game->statTracker.holds), 0, 7*counter++, 2);
    }

    if(game->gameMode == MASTER){
        aprints("Section Cools: " + std::to_string(game->coolCount), 0, 7*counter++, 2);
        aprints("Section Regrets: " + std::to_string(game->regretCount), 0, 7*counter++, 2);
    }

    if(game->statTracker.maxZonedLines > 0){
        aprints("Max MultiClear: " + std::to_string(game->statTracker.maxZonedLines), 0, 7*counter++, 2);
    }
}

int pauseMenu(){
    int selection = 0;
    int maxSelection;

    int optionsHeight = 10;
    int optionsCounter = 0;

    clearText();
    setSmallTextArea(110, 1, 1, 10, 20);

    int prevBld = blendInfo;
    enableBlend((1 << 6) + (0b11101 << 9) + (1));
    setTiles(25,0,32*32,tileBuild(34, false, false, 0));

    drawFrame(1);
    if(game->zoneTimer)
        frameSnow(1);

    setLayerScroll(0, 0, 0);

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

    //calculate pps
    FIXED t = gameSeconds * float2fx(0.0167f);
    FIXED pps = 0;
    if(t > 0)
        pps =  fxdiv(int2fx(game->pieceCounter),(t));

    std::string ppsStr = std::to_string(fx2int(pps)) + ".";

    int fractional = pps & 0xff;
    for(int i = 0; i < 2; i++){
        fractional *= 10;
        ppsStr += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

    std::string totalTime = timeToString(gameSeconds);
    igt = game->inGameTimer;

    showModeText();

    bool shown = false;

    bool showSleep = false;

    #ifdef GBA
    showSleep = true;
    #endif

    while (closed()) {
        if (!onStates){
            maxSelection = 5;
            if(game->gameMode != TRAINING)
                maxSelection--;
            if(!showSleep)
                maxSelection--;

            if(maxSelection == 3){
                optionsHeight = 11;
            }
        } else
            maxSelection = 3;

        vsync();
        key_poll();

        for (int i = 0; i < maxSelection; i++)
            aprint(" ", 10, optionsHeight + 2 * i);
        aprint(">", 10, optionsHeight + 2 * selection);

        aprint("PAUSE!", 12, 4);

        if (!onStates) {
            optionsCounter = 0;
            aprint("Resume", 12, optionsHeight + optionsCounter++ * 2);
            aprint("Restart", 12, optionsHeight + optionsCounter++ * 2);

            if(game->gameMode == TRAINING)
                aprint("Saves", 12, optionsHeight + optionsCounter++ * 2);

            if(showSleep)
                aprint("Sleep", 12, optionsHeight + optionsCounter++ * 2);

            aprint("Quit", 12, optionsHeight + optionsCounter++ * 2);

            if (key_hit(KEY_A)) {
                int n = selection;
                if(!showSleep && selection >= 2)
                    n++;
                if(!(game->gameMode == TRAINING) && selection >= 2)
                    n++;


                if (n == 0) {
                    sfx(SFX_MENUCONFIRM);
                    clearText();
                    // updateText();
                    paused = false;
                    resumeSong();
                    break;
                } else if (n == 1) {
                    // restart = true;
                    startGame();
                    changeScene(new GameScene());
                    paused = false;
                    sfx(SFX_MENUCONFIRM);
                    break;
                } else if (n == 2) {
                    onStates = true;
                    selection = 0;
                    // update();
                    clearText();
                } else if (n == 3) {
                    sleep();
                    showStats(showingStats, totalTime, ppsStr);
                    showModeText();
                } else if (n == 4) {
                    enableBlend(prevBld);
                    sfx(SFX_MENUCANCEL);
                    changeScene(new MainMenuScene(), Transitions::FADE);
                    return 1;
                }
            }
        } else {
            optionsCounter = 0;
            aprintColor("Load", 12, optionsHeight + optionsCounter++ * 2, !(saveExists));
            aprint("Save", 12, optionsHeight + optionsCounter++ * 2);
            aprint("Back", 12, optionsHeight + optionsCounter++ * 2);

            if (key_hit(KEY_A)) {
                if (selection == 0) {
                    if (saveExists) {
                        delete game;
                        game = new Game(*quickSave);
                        game->setTuning(getTuning());
                        floatingList.clear();
                        placeEffectList.clear();
                        clearGlow();

                        if(savefile->settings.colors == 3)
                            setPalette();

                        if (enableBot) {
                            delete testBot;
                            testBot = new Bot(game);
                        }

                        aprint("Loaded!", 22, 18);
                        sfx(SFX_MENUCONFIRM);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else if (selection == 1) {
                    if(saveExists)
                        delete quickSave;
                    quickSave = new Game(*game);
                    saveExists = true;

                    aprint(" Saved!", 22, 18);
                    sfx(SFX_MENUCONFIRM);
                } else if (selection == 2) {
                    sfx(SFX_MENUCONFIRM);
                    clearText();
                    // updateText();
                    onStates = false;
                    selection = 0;
                    break;
                }
            }
        }

        if (key_hit(KEY_START) || key_hit(KEY_SELECT)) {
            sfx(SFX_MENUCONFIRM);
            clearText();
            // updateText();
            paused = false;
            resumeSong();
            break;
        }

        if (key_hit(KEY_B)) {
            sfx(SFX_MENUCONFIRM);
            if(onStates){
                clearText();
                onStates = false;
                selection = 0;
                // updateText();
            }else{
                clearText();
                // updateText();
                paused = false;
                resumeSong();
            }
            break;
        }

        if (key_hit(KEY_UP)) {
            if (selection == 0)
                selection = maxSelection - 1;
            else
                selection--;
            sfx(SFX_MENUMOVE);
        }

        if (key_hit(KEY_DOWN)) {
            if (selection == maxSelection - 1)
                selection = 0;
            else
                selection++;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_L) && !onStates){
            sfx(SFX_MENUCONFIRM);
            showingStats = !showingStats;
            clearSmallText();
            showStats(showingStats, totalTime, ppsStr);
        }

        if(!shown && !onStates){
            shown = true;
            showStats(showingStats, totalTime, ppsStr);
        }

        showSprites(128);
    }

    enableBlend(prevBld);

    clearTilemap(25);
    clearTilemap(26);

    drawFrame(0);
    if(game->zoneTimer)
        frameSnow(0);

    showBackground(0);
    setSmallTextArea(110, 3, 7, 9, 10);

    return 0;
}

int onRecord() {
    int place = -1;

    if(bigMode)
        return place;

    for (int i = 0; i < 5; i++) {
        if (game->gameMode == MARATHON) {
            if(subMode == 0){
                if (game->score < savefile->marathon[mode].highscores[i].score)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->marathon[mode].highscores[j + 1] = savefile->marathon[mode].highscores[j];

                std::string name = nameInput(i);

                savefile->marathon[mode].highscores[i].score = game->score;
                strncpy(savefile->marathon[mode].highscores[i].name, name.c_str(), 9);
            }else{
                if (game->score < savefile->zone[mode].highscores[i].score)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->zone[mode].highscores[j + 1] = savefile->zone[mode].highscores[j];

                std::string name = nameInput(i);

                savefile->zone[mode].highscores[i].score = game->score;
                strncpy(savefile->zone[mode].highscores[i].name, name.c_str(), 9);
            }
        } else if (game->gameMode == SPRINT && game->won == 1) {
            if(subMode == 0){
                if (gameSeconds > savefile->sprint[mode].times[i].frames && savefile->sprint[mode].times[i].frames)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->sprint[mode].times[j + 1] = savefile->sprint[mode].times[j];

                std::string name = nameInput(i);

                savefile->sprint[mode].times[i].frames = gameSeconds;
                strncpy(savefile->sprint[mode].times[i].name, name.c_str(), 9);
            }else{
                if (gameSeconds > savefile->sprintAttack[mode].times[i].frames && savefile->sprintAttack[mode].times[i].frames)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->sprintAttack[mode].times[j + 1] = savefile->sprintAttack[mode].times[j];

                std::string name = nameInput(i);

                savefile->sprintAttack[mode].times[i].frames = gameSeconds;
                strncpy(savefile->sprintAttack[mode].times[i].name, name.c_str(), 9);
            }
        } else if (game->gameMode == DIG && game->won == 1) {
            if(subMode == 0){
                if (gameSeconds > savefile->dig[mode].times[i].frames && savefile->dig[mode].times[i].frames)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->dig[mode].times[j + 1] = savefile->dig[mode].times[j];

                std::string name = nameInput(i);

                savefile->dig[mode].times[i].frames = gameSeconds;
                strncpy(savefile->dig[mode].times[i].name, name.c_str(), 9);
            }else{
                if (game->pieceCounter > savefile->digEfficiency[mode].highscores[i].score && savefile->digEfficiency[mode].highscores[i].score)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->digEfficiency[mode].highscores[j + 1] = savefile->digEfficiency[mode].highscores[j];

                std::string name = nameInput(i);

                savefile->digEfficiency[mode].highscores[i].score = game->pieceCounter;
                strncpy(savefile->digEfficiency[mode].highscores[i].name, name.c_str(), 9);
            }
        } else if (game->gameMode == ULTRA) {
            if (game->score < savefile->ultra[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->ultra[mode].highscores[j + 1] = savefile->ultra[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->ultra[mode].highscores[i].score = game->score;
            strncpy(savefile->ultra[mode].highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == BLITZ) {
            if (game->score < savefile->blitz[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->blitz[mode].highscores[j + 1] = savefile->blitz[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->blitz[mode].highscores[i].score = game->score;
            strncpy(savefile->blitz[mode].highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == COMBO) {
            if (game->linesCleared < savefile->combo.highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->combo.highscores[j + 1] = savefile->combo.highscores[j];

            std::string name = nameInput(i);

            savefile->combo.highscores[i].score = game->linesCleared;
            strncpy(savefile->combo.highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == SURVIVAL) {
            if (gameSeconds < savefile->survival[mode].times[i].frames && savefile->survival[mode].times[i].frames)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->survival[mode].times[j + 1] = savefile->survival[mode].times[j];

            std::string name = nameInput(i);

            savefile->survival[mode].times[i].frames = gameSeconds;
            strncpy(savefile->survival[mode].times[i].name, name.c_str(), 9);
        } else if (game->gameMode == CLASSIC) {
            if (game->score < savefile->classic[subMode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->classic[subMode].highscores[j + 1] = savefile->classic[subMode].highscores[j];

            std::string name = nameInput(i);

            savefile->classic[subMode].highscores[i].score = game->score;
            strncpy(savefile->classic[subMode].highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == MASTER) {
            int grade = game->grade + game->coolCount + (int) game->creditGrade;

            if(grade > 32)
                grade = 32;

            if (grade < savefile->master[subMode].grade[i] || (grade == savefile->master[subMode].grade[i] && gameSeconds > savefile->master[subMode].times[i].frames))
                continue;

            for (int j = 3; j >= i; j--){
                savefile->master[subMode].times[j + 1] = savefile->master[subMode].times[j];
                savefile->master[subMode].grade[j + 1] = savefile->master[subMode].grade[j];
            }

            std::string name = nameInput(i);

            savefile->master[subMode].times[i].frames = gameSeconds;
            strncpy(savefile->master[subMode].times[i].name, name.c_str(), 9);
            savefile->master[subMode].grade[i] = grade;
        }

        place = i;

        saveSavefile();
        // saveToSram();
        break;
    }

    return place;
}

void showModeText(){
    if(game->gameMode > 0 && game->gameMode <= 11){
        int counter = 0;
        std::string str;
        std::string str2;

        str = modeStrings[game->gameMode-1];

        if(game->gameMode == TRAINING)
            str = "Training";

        aprintColor(str,30-str.size(),counter++,0);

        str = "";
        if(game->gameMode == TRAINING){
            if(game->trainingMode)
                str = "Finesse";
        }else{
            if(game->gameMode != CLASSIC && game->gameMode != MASTER)
                str = modeOptionStrings[game->gameMode-1][mode];
            else if(game->gameMode == MASTER)
                str = modeOptionStrings[game->gameMode-1][subMode];
            else
                str = modeOptionStrings[game->gameMode-1][0];
        }

        if(str != "")
            aprintColor(str,30-str.size(),counter++,0);

        str = "";
        str2 = "";
        if(game->subMode){
            switch(game->gameMode){
            case MARATHON: str = "Zone"; break;
            case SPRINT: str = "Attack"; break;
            case DIG: str = "Efficiency"; break;
            case CLASSIC:
                str = "B-Type";
                str2 = std::to_string(initialLevel) + "-" + std::to_string(game->bTypeHeight);
                break;
            }
        }else{
            if(game->gameMode == CLASSIC)
                str = "A-Type";
        }

        if(str != "")
            aprintColor(str,30-str.size(),counter++,0);
        if(str2 != "")
            aprintColor(str2,30-str2.size(),counter++,0);

        if(bigMode)
            aprintColor("BIG MODE",22,counter++,0);
    }
}

Replay * test = nullptr;
// ReplayHeader test;

void saveReplay(){
    if(replaying)
        return;

    const int len = game->replay.size();
    if(len >= 4096){
        log("replay size too big (" + std::to_string(len) + ")");
        return;
    }

    if(!game->replayElligible){
        log("replay not elligible");
        return;
    }

    if(test != nullptr)
        delete test;

    test = new Replay();

    test->header.tag = savefile->newGame;
    test->header.length = len;
    test->header.duration = game->timer;
    test->header.seed = game->initSeed;

    test->header.options = *previousGameOptions;

    auto it = game->replay.begin();
    for(int i = 0; i < len; i++){
        Timestamp *move = *it;

        test->moves[i] = (((move->timer & 0x3fffff) << 11) + ((move->dir & 1) << 9) + (move->move & 0x1ff));

        ++it;
    }

    #ifdef PC
    std::ofstream output("replay.sav",std::ios::binary | std::ios::out);

    char * src = (char * ) test;
    output.write(src, sizeof(Replay));

    if(!output){
        log("Error when trying to write replay.");
        return;
    }else{
        log("Wrote save to file.");
    }

    output.close();
    #endif
}

ReplayHeader *curRepHeader = nullptr;
std::list<Timestamp*> currentReplay;

void loadReplay(){
    currentReplay.clear();

    if(test != nullptr){
        if(curRepHeader == nullptr)
            curRepHeader = new ReplayHeader();

        memcpy32(curRepHeader,&test->header,sizeof(ReplayHeader) / 4);

        for(int i = 0; i < (int)test->header.length; i++){
            currentReplay.push_back(new Timestamp(test->moves[i]));
        }

        startGame(curRepHeader->options, curRepHeader->seed);
        replaying = true;
        replayIterator = currentReplay.begin();

        gameLoop();
        return;
    }

    #ifdef PC

    std::ifstream input("replay.sav",std::ios::binary | std::ios::in);

    Replay rep;
    char * dst = (char * ) &rep;

    log(std::to_string(sizeof(Replay)));

    input.read(dst,sizeof(Replay));

    input.close();

    if(!input){
        log("Error when trying to load replay.");
        return;
    }else{
        log("Found replay file.");
    }

    if(rep.header.tag != savefile->newGame){
        log("Invalid replay (old version, tag: " + std::to_string(rep.header.tag) + " ).");
        return;
    }else{
        log("Correct version number.");
    }

    if(rep.header.length > 4096){
        log("Invalid replay (too large).");
        return;
    }

    log("Attemping to load moves...");
    for(int i = 0; i < (int)rep.header.length; i++){
        currentReplay.push_back(new Timestamp(rep.moves[i]));
    }
    log("Loaded moves.");

    if(curRepHeader == nullptr)
        curRepHeader = new ReplayHeader();

    memcpy32(curRepHeader,&rep.header,sizeof(ReplayHeader) / 4);

    replaying = true;
    replayIterator = currentReplay.begin();

    log("trying to start game...");
    startGame(curRepHeader->options,curRepHeader->seed);

    gameLoop();

    #endif
}
