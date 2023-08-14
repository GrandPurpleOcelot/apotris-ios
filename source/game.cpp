#include "def.h"
#include "LinkConnection.h"
#include "menu.h"
#include "meter1_tiles_bin.h"
// #include "soundbank.h"
#include "platform.hpp"
#include "sprite36tiles_bin.h"
#include "sprite5tiles_bin.h"
#include "sprites.h"
#include "blockEngine.h"
#include "tetromino.hpp"
#include "logging.h"
#include <string>
#include <tuple>
#include "text.h"

#include "rumble.h"

#include "classic_pal_bin.h"
#include "logging.h"
#include "posprintf.h"
#include "scene.hpp"

using namespace BlockEngine;

void diagnose();
void clearGlow();
void addGlow(Drop);
void addPlaceEffect(Drop);
void drawFrame(int layer);
void drawFrameBackgrounds();
void drawGrid();
void progressBar(int layer);
void showBar(int, int, int, int, int);
void showBestMove();
bool checkDiagonal(int);
void showSpeedMeter(int);
void hideMinos();
void disappear();
INLINE int getBoard(int,int);
void zoneFlash();
void resetZonePalette();
void rainbowPalette();
void frameSnow(int layer);
void showZoneText();
void showFullMeter();
Function getActionFromKey(int key);
void liftKeys();
// void journeyFlash();
void setJourneyGraphics(Save * save, int level);
void replayControl();
void showBackgroundBig(int offset);
INLINE int prep(int n);

IWRAM_CODE void updateGrid();

Game* game = nullptr;
OBJ_ATTR* pawnSprite;
OBJ_ATTR* pawnShadow;
OBJ_ATTR* holdSprite;
OBJ_ATTR* holdFrameSprite;
OBJ_ATTR* queueFrameSprites[3];

OBJ_ATTR* queueSprites[5];

OBJ_ATTR* moveSprites[3];

bool onStates = false;

int clearTimer = 0;

int maxClearTimer = 20;

std::string clearTypeText = "";
#define maxClearTextTimer 100
#define clearTextHeight 15

s16 glow[20][10];

int push = 0;
int pushTimer = 0;

int restartTimer = 0;
#define maxRestartTimer 20

int attackFlashTimer = 0;
#define attackFlashMax  10

int rumbleTimer = 0;
#define rumbleMax 1

std::list<FloatText> floatingList;

std::list<Effect> effectList;

std::list<PlaceEffect> placeEffectList;

#define eventPauseTimerMax 60

int eventPauseTimer = 0;

BlockEngine::Drop latestDrop;

static bool holdingSave = false;

#define flashTimerMax 16
static int flashTimer = 0;

COLOR * previousPalette = nullptr;

static int rainbowTimer = 1;
static bool rainbowIncreasing = 0;

static u16 rainbow[5];

static bool creditRefresh = false;
static u16 fullMeterTimer = 0;
#define fullMeterTimerMax 5 * 60;
#define fullMeterAnimationLength 15

bool enableBot = false;
static bool refreshSkin = false;
// Bot *testBot;

FIXED shakeBuff;
FIXED shakeVelocity;

bool replaying = false;
bool exitGame = false;

int current[22][12];
int previous[22][12];

int gridUpdateTimer = 0;
int gridUpdateTimerMax = 4;

int damp = 2;

int dampTimer = 0;
int dampTimerMax = 60;

void GameScene::draw(){
    // std::string prof;
    // profile_start();
    if (control())
        return;

    if(paused)
        return;

    checkSounds();

    showPawn();
    showShadow();

    showHold();
    showQueue();

    drawGrid();//10k idle, 20k when clearing

    screenShake();//~320

    showComboStreak();
    showClearText();//~1200

    showPlaceEffect();// 300 idle, 3600 per effect when on

    if(game->refresh)
        updateText();

    showSprites(128);

    if (game->refresh) {
        showBackground(0);
        game->resetRefresh();
    }else if (game->clearLock && !eventPauseTimer){
        showBackground(0);
        showTimer();
    }else{
        showTimer();
    }

}

void GameScene::updateText() {
    if(!game->zoneTimer)
        showText();
    else{
        showZoneText();
    }

    showTimer();
    showClearText();

    if(proMode){
        showFinesseCombo();
        showPPS();
    }
}

void checkSounds() {
    if (game->sounds.hold)
        sfx(SFX_HOLD);
    if (game->sounds.shift)
        sfx(SFX_SHIFT2);
    if (game->sounds.place) {
        sfx(SFX_PLACE);

        #ifdef GBA
        shake = (shakeMax * (savefile->settings.shakeAmount) / 4) /2;
        #endif

        #ifdef PC
        shakeVelocity = int2fx(((shakeMax * (savefile->settings.shakeAmount) / 4))/2) * 2;
        #endif

        if(!game->sounds.finesse && game->trainingMode)
            for(int i = 0; i < 3; i++)
                sprite_hide(moveSprites[i]);

        // rumbleTimer = rumbleMax * savefile->settings.rumble;
    }

    if (game->sounds.invalid){
        sfx(SFX_INVALID);
        rumbleTimer = rumbleMax * savefile->settings.rumble;
    }

    if (game->sounds.rotate)
        sfx(SFX_ROTATE);
    if (game->sounds.finesse){
        if(game->trainingMode)
            showBestMove();

        sfx(SFX_MENUCANCEL);
    }

    if (game->sounds.clear) {

        if(game->previousClear.linesCleared <= 4){
            int speed = game->comboCounter - 1;
            if (speed > 10)
                speed = 10;

            sfxRate(SFX_LEVELUP,1.0 + (float)speed/10);
        }else{
            sfx(SFX_MULTICLEAR);
        }

        int soundEffect = -1;

        clearTypeText = "";
        if(game->zoneTimer){

        } else if (game->previousClear.isPerfectClear == 1) {
            soundEffect = SFX_PERFECTCLEAR;
            clearTypeText = "perfect clear";
            effectList.push_back(Effect(0));
        } else if (game->previousClear.isTSpin == 2) {
            if (game->previousClear.isBackToBack == 1)
                soundEffect = SFX_BACKTOBACKTSPIN;
            else
                soundEffect = SFX_TSPIN;

            if (game->previousClear.linesCleared == 1) {
                clearTypeText = "t-spin single";
            } else if (game->previousClear.linesCleared == 2) {
                clearTypeText = "t-spin double";
            } else if (game->previousClear.linesCleared == 3) {
                clearTypeText = "t-spin triple";
            }
        } else if (game->previousClear.isTSpin == 1) {
            soundEffect = SFX_TSPINMINI;

            clearTypeText = "t-spin mini";
        } else if (game->previousClear.linesCleared > 4) {
            int n = game->previousClear.linesCleared;
            if(n < 8){
                clearTypeText = "quad";
                soundEffect = SFX_QUAD;
            }else if(n < 12){
                clearTypeText = "octo";
                soundEffect = SFX_OCTORIS;
            }else if(n < 16){
                clearTypeText = "dodeca";
                soundEffect = SFX_DODECATRIS;
            }else if(n < 18){
                clearTypeText = "decahexa";
                soundEffect = SFX_DECAHEXATRIS;
            }else if(n < 20){
                clearTypeText = "perfectus";
                soundEffect = SFX_PERFECTRIS;
            }else{
                clearTypeText = "ultimus";
                soundEffect = SFX_ULTIMATRIS;
            }
        } else if (game->previousClear.linesCleared == 4) {
            if (game->previousClear.isBackToBack == 1)
                soundEffect = SFX_BACKTOBACKQUAD;
            else
                soundEffect = SFX_QUAD;
            clearTypeText = "quad";
        } else if (game->previousClear.linesCleared == 3) {
            soundEffect = SFX_TRIPLE;
            clearTypeText = "triple";
        } else if (game->previousClear.linesCleared == 2) {
            soundEffect = SFX_DOUBLE;
            clearTypeText = "double";
        }

        if (savefile->settings.floatText && clearTypeText != "")
            floatingList.push_back(FloatText(clearTypeText));

        if (savefile->settings.announcer && soundEffect != -1)
            sfx(soundEffect);
    }else if(game->sounds.tspin){
        if(game->previousClear.isTSpin == 1){
            clearTypeText = "t-spin mini";
            sfx(SFX_TSPINMINI);
        }else{
            clearTypeText = "t-spin";
            sfx(SFX_TSPIN);
        }

        if (savefile->settings.floatText)
            floatingList.push_back(FloatText(clearTypeText));
    }

    if (game->sounds.levelUp) {
        sfx(SFX_LEVELUPSOUND);

        if(savefile->settings.colors == 3){
            int n = getClassicPalette();

            for(int i = 0; i < 8; i++){
                loadPalette(i, 1, &nesPalette[n][0], 4);
                loadPalette(i+16, 1, &nesPalette[n][0], 4);
            }
        }

        if(game->gameMode == MASTER){
            maxClearTimer = game->maxClearDelay;
        } else if(game->gameMode == MARATHON){
            flashTimer = flashTimerMax;
            setJourneyGraphics(savefile, game->level);
            setPalette();
            zoneFlash();
            refreshSkin = true;
        }
    }

    if(game->sounds.initial){
        // sfx(SFX_INITIAL);
    }

    std::string sectionText = "";

    switch(game->sounds.section){
        case -1:
            sectionText = "regret!!";
            sfx(SFX_REGRET);
            break;
        case 1:
            sectionText = "cool!!";
            sfx(SFX_COOL);
            break;
        case 2: sfx(SFX_SECRET); break;
    }

    if (sectionText.size() && savefile->settings.floatText)
        floatingList.push_back(FloatText(sectionText));

    if(game->sounds.disappear){
        disappear();
    }

    if (game->sounds.zone == 1) {
        clearText();
        gradient(false);

        holdingSave = true;

        previousSettings = savefile->settings;

        savefile->settings.colors = 4;
        savefile->settings.palette = 7;
        savefile->settings.clearEffect = 2;
        setPalette();
        setClearEffect();
        showBackground(0);

        flashTimer = flashTimerMax;
        zoneFlash();

        for(int i = 0; i < 20; i++){
            setTile(25, 9, i, tileBuild(4, false, false, 0));
            setTile(25, 20, i, tileBuild(4, true, false, 0));
        }

        if (savefile->settings.lightMode){
            for(int i = 0; i < 4; i++){
                COLOR c = RGB15(10-i, 10-i, 10-i);
                addColorToPalette(i, 5, c, 1);
            }
        }

        setMusicTempo(512);
        setMusicVolume(512 * ((float)savefile->settings.volume / 20));

        sfx(SFX_ZONESTART);
    } else if (game->sounds.zone == 2) {
        savefile->settings.lightMode = !previousSettings.lightMode;

        setPalette();
        if (savefile->settings.lightMode){
            for(int i = 0; i < 4; i++){
                COLOR c = RGB15(10-i, 10-i, 10-i);
                addColorToPalette(i, 5, c, 1);
            }
        }

        // if(savefile->settings.lightMode)
        //     memset16(&pal_bg_mem[4 * 16 + 5], 0x4a52, 1);

    } else if (game->sounds.zone == -1) {
        aprintClearArea(10, 0, 10, 20);
        resetZonePalette();

        flashTimer = flashTimerMax;
        eventPauseTimer = flashTimerMax + 2;

        //FIXME:
        //zoneFlash();
    }

    if(game->sounds.meter){
        fullMeterTimer = fullMeterAnimationLength;
    }

    if(game->gameMode == MARATHON && game->subMode){
        showFullMeter();
    }

    game->resetSounds();
}

int edges[16] = {
    0x0000,//none
    0x080f,
    0x0410,
    0x0c16,
    0x0010,
    0x0816,
    0x0011,
    0x0817,
    0x000f,
    0x0012,
    0x0416,
    0x0418,
    0x0016,
    0x0018,
    0x0017,
    0x0019,//all
};

void showBackground(int offset) {
    bool showEdges = savefile->settings.edges;

    bool up, down, left, right;
    bool before = false, after = false;

    int startY = 20;

    const int clearTile = tileBuild(3, false, false, savefile->settings.lightMode);

    #ifdef PC
    startY = 1;
    #endif

    if(!game->pawn.big){
        for (int i = startY; i < game->lengthY && (i - 20 + offset) < 20; i++) {
            int y = i - 20 + offset;

            if(game->bitboard[i] == 0 && !(i < game->lengthY - 1 && game->bitboard[i+1] != 0)){
                for(int j = 0; j < 10; j++)
                    setTile(25, 10+j, y, 0);
                continue;
            }

            if(game->clearLock){
                before = game->lineClearArray[i - 1];

                after = (i < game->lengthY-1 && game->lineClearArray[i + 1]);
            }

            for(int j = 0; j < game->lengthX; j++){
                int b = game->board[i][j];

                if(!b || game->lineClearArray[i] || game->disappearTimers[i][j] == 1){
                    if (!showEdges) {
                        setTile(25, 10+j, y, 0);
                        continue;
                    }

                    if(!game->disappearing){
                        up = (game->board[i - 1][j] > 0 && !before);
                        left = (j - 1 >= 0 && game->board[i][j - 1] > 0 && !(game->lineClearArray[i]));
                        right = (j + 1 <= 9 && game->board[i][j + 1] > 0 && !(game->lineClearArray[i]));
                        down = (i + 1 <= 39 && game->board[i + 1][j] > 0 && !after);
                    }else{
                        up = (getBoard(j,i-1) > 0 && !before);
                        left = (j - 1 >= 0 && getBoard(j-1, i) > 0 && !(game->lineClearArray[i]));
                        right = (j + 1 <= 9 && getBoard(j+1,i) > 0 && !(game->lineClearArray[i]));
                        down = (i + 1 <= 39 && getBoard(j,i+1) > 0 && !after);
                    }

                    int count = (up << 3) + (left << 2) + (right << 1) + down;

                    setTile(25, 10+j, y, tileBuild(edges[count] + (savefile->settings.lightMode * 0x1000)));
                    continue;
                }

                int offset = 1;

                int n = (b - 1) & 0xf;

                if(savefile->settings.skin == 7 || savefile->settings.skin == 8)
                    offset = 48 + (n);
                else if(savefile->settings.skin >= 11)
                    offset = 128 + GameInfo::connectedConversion[b >> 4];

                if(n != 8)
                    setTile(25, 10+j, y, tileBuild(offset, false, false, n * !game->zoneTimer));
                else
                    setTile(25, 10+j, y, tileBuild(3, false, false, 0));

            }
        }

        for(auto const& i : game->linesToClear){
            int y = i - 20 + offset;

            if(savefile->settings.clearEffect < 3){
                for(int j = 0; j < 5; j++){
                    if (clearTimer < maxClearTimer - 10 + j * 2)
                        setTile(25, 10+j, y, clearTile);
                }
                for(int j = 5; j < 10; j++){
                    if (clearTimer < maxClearTimer - 10 + (9 - j) * 2)
                        setTile(25, 10+j, y, clearTile);
                }
            }else if(savefile->settings.clearEffect == 3){
                for(int j = 0; j < 5; j++){
                    if (clearTimer < maxClearTimer - 10 + (4 - j) * 2)
                        setTile(25, 10+j, y, clearTile);
                }
                for(int j = 5; j < 10; j++){
                    if (clearTimer < maxClearTimer - 10 + (j-5) * 2)
                        setTile(25, 10+j, y, clearTile);
                }
            }
        }

    }else{
        for (int i = startY; i < 40 && (i - 20 + offset) < 20; i++) {
            int y = i - 20 + offset;

            if(game->bitboard[i/2] == 0 && !(i/2 < game->lengthY - 1 && game->bitboard[i/2+1] != 0)){
                for(int j = 0; j < 10; j++)
                    setTile(25, 10+j, y, 0);
                continue;
            }

            if(game->clearLock){
                before = game->lineClearArray[(i-1)/2];

                after = ((i+1)/2 < game->lengthY && game->lineClearArray[(i+1)/2]);
            }

            for(int j = 0; j < 10; j++){
                int b = game->board[i/2][j/2];

                if(!b || game->lineClearArray[i/2] || game->disappearTimers[i/2][j/2] == 1){
                    if (!showEdges) {
                        setTile(25, 10+j, y, 0);
                        continue;
                    }

                    if(!game->disappearing){
                        up = (game->board[(i-1)/2][j/2] > 0 && !before);
                        left = ((j-1)/2 >= 0 && game->board[i/2][(j-1)/2] > 0 && !(game->lineClearArray[i/2]));
                        right = ((j+1)/2 < game->lengthX && game->board[i/2][(j+1)/2] > 0 && !(game->lineClearArray[i/2]));
                        down = ((i+1)/2 < game->lengthY && game->board[(i+1)/2][j/2] > 0 && !after);
                    }else{
                        up = (getBoard(j/2, (i-1)/2) > 0 && !before);
                        left = ((j-1)/2 >= 0 && getBoard((j-1)/2, i/2) > 0 && !(game->lineClearArray[i/2]));
                        right = ((j+1)/2 < game->lengthX && getBoard((j+1)/2, i/2) > 0 && !(game->lineClearArray[i/2]));
                        down = ((i+1)/2 < game->lengthY && getBoard(j/2, (i+1)/2) > 0 && !after);
                    }

                    int count = (up << 3) + (left << 2) + (right << 1) + down;

                    setTile(25, 10+j, y, tileBuild(edges[count] + (savefile->settings.lightMode * 0x1000)));
                    continue;
                }

                int offset = 1;

                int n = (b - 1) & 0xf;

                if(savefile->settings.skin == 7 || savefile->settings.skin == 8)
                    offset = 48 + (n);
                else if(savefile->settings.skin >= 11)
                    offset = 128 + GameInfo::connectedConversion[b >> 4];

                if(n != 8)
                    setTile(25, 10+j, y, tileBuild(offset, false, false, n * !game->zoneTimer));
                else
                    setTile(25, 10+j, y, tileBuild(3, false, false, 0));

            }
        }

        for(auto const& line : game->linesToClear){
            int l = line*2 - 20 + offset;

            for(int y = l; y < l+2; y++){
                if(savefile->settings.clearEffect < 3){
                    for(int j = 0; j < 5; j++){
                        if (clearTimer < maxClearTimer - 10 + j * 2)
                            setTile(25, 10+j, y, clearTile);
                    }
                    for(int j = 5; j < 10; j++){
                        if (clearTimer < maxClearTimer - 10 + (9 - j) * 2)
                            setTile(25, 10+j, y, clearTile);
                    }
                }else if(savefile->settings.clearEffect == 3){
                    for(int j = 0; j < 5; j++){
                        if (clearTimer < maxClearTimer - 10 + (4 - j) * 2)
                            setTile(25, 10+j, y, clearTile);
                    }
                    for(int j = 5; j < 10; j++){
                        if (clearTimer < maxClearTimer - 10 + (j-5) * 2)
                            setTile(25, 10+j, y, clearTile);
                    }
                }
            }
        }
    }
}

void showBackgroundBig(int offset){

    bool showEdges = savefile->settings.edges;

    bool up, down, left, right;
    bool before = false, after = false;

    int startY = 20;

    #ifdef PC
    startY = 1;
    #endif

    for (int i = startY; i < 40 && (i - 20 + offset) < 20; i++) {
        int y = i - 20 + offset;

        if(game->bitboard[i/2] == 0 && !(i/2 < game->lengthY - 1 && game->bitboard[i/2+1] != 0)){
            for(int j = 0; j < 10; j++)
                setTile(25, 10+j, y, 0);
            continue;
        }

        if(game->clearLock){
            before = game->lineClearArray[(i-1)/2];

            after = ((i+1)/2 < game->lengthY && game->lineClearArray[(i+1)/2]);
        }

        for(int j = 0; j < 10; j++){
            int b = game->board[i/2][j/2];

            if(!b || game->lineClearArray[i/2] || game->disappearTimers[i/2][j/2] == 1){
                if (!showEdges) {
                    setTile(25, 10+j, y, 0);
                    continue;
                }

                if(!game->disappearing){
                    up = (game->board[(i-1)/2][j/2] > 0 && !before);
                    left = ((j-1)/2 >= 0 && game->board[i/2][(j-1)/2] > 0 && !(game->lineClearArray[i/2]));
                    right = ((j+1)/2 < game->lengthX && game->board[i/2][(j+1)/2] > 0 && !(game->lineClearArray[i/2]));
                    down = ((i+1)/2 < game->lengthY && game->board[(i+1)/2][j/2] > 0 && !after);
                }else{
                    up = (getBoard(j/2, (i-1)/2) > 0 && !before);
                    left = ((j-1)/2 >= 0 && getBoard((j-1)/2, i/2) > 0 && !(game->lineClearArray[i/2]));
                    right = ((j+1)/2 < game->lengthX && getBoard((j+1)/2, i/2) > 0 && !(game->lineClearArray[i/2]));
                    down = ((i+1)/2 < game->lengthY && getBoard(j/2, (i+1)/2) > 0 && !after);
                }

                int count = (up << 3) + (left << 2) + (right << 1) + down;

                setTile(25, 10+j, y, tileBuild(edges[count] + (savefile->settings.lightMode * 0x1000)));
                continue;
            }

            int offset = 1;

            int n = (b - 1) & 0xf;

            if(savefile->settings.skin == 7 || savefile->settings.skin == 8)
                offset = 48 + (n);
            else if(savefile->settings.skin >= 11)
                offset = 128 + GameInfo::connectedConversion[b >> 4];

            if(n != 8)
                setTile(25, 10+j, y, tileBuild(offset, false, false, n * !game->zoneTimer));
            else
                setTile(25, 10+j, y, tileBuild(3, false, false, 0));

        }
    }

    const int clearTile = tileBuild(3, false, false, savefile->settings.lightMode);

    for(auto const& line : game->linesToClear){

        int l = line*2 - 20 + offset;
        for(int y = l; y < l+2; y++){
            if(savefile->settings.clearEffect < 3){
                for(int j = 0; j < 5; j++){
                    if (clearTimer < maxClearTimer - 10 + j * 2)
                        setTile(25, 10+j, y, clearTile);
                }
                for(int j = 5; j < 10; j++){
                    if (clearTimer < maxClearTimer - 10 + (9 - j) * 2)
                        setTile(25, 10+j, y, clearTile);
                }
            }else if(savefile->settings.clearEffect == 3){
                for(int j = 0; j < 5; j++){
                    if (clearTimer < maxClearTimer - 10 + (4 - j) * 2)
                        setTile(25, 10+j, y, clearTile);
                }
                for(int j = 5; j < 10; j++){
                    if (clearTimer < maxClearTimer - 10 + (j-5) * 2)
                        setTile(25, 10+j, y, clearTile);
                }
            }
        }
    }
}

void showPawn() {
    pawnSprite = &obj_buffer[112];
    // if (game->clearLock || game->pawn.current == -1 || (enableBot && game->gameMode != BATTLE)) {
    if (game->clearLock || game->activePiece == -1) {
        sprite_hide(pawnSprite);
        return;
    }

    #ifdef PC
    if(flashTimer){
        sprite_hide(pawnSprite);
        return;
    }
    #endif

    sprite_unhide(pawnSprite, 0);

    int * b = BlockEngine::getShape(game->activePiece, game->pawn.rotation, game->rotationSystem);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int n = b[i * 4 + j];
            if (n > 0){
                if(savefile->settings.skin == 11)
                    setSpriteTile(16 * 7, j, i, 4, &sprite38tiles_bin[GameInfo::connectedConversion[(n)>>4] * 32]);
                else if(savefile->settings.skin == 12)
                    setSpriteTile(16 * 7, j, i, 4, &sprite39tiles_bin[GameInfo::connectedConversion[(n)>>4] * 32]);
                else if(savefile->settings.skin == 13)
                    setSpriteTile(16 * 7, j, i, 4, &sprite40tiles_bin[GameInfo::connectedConversion[(n)>>4] * 32]);
                else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                    setSpriteTile(16 * 7, j, i, 4, blockSprite);
                else
                    setSpriteTile(16 * 7, j, i, 4, classicTiles[savefile->settings.skin-7][game->activePiece]);
            }else{
                clearSpriteTile(16*7,j,i,4);
            }
        }
    }

    delete[] b;

    int n = game->activePiece;

    int blend = 0;

    if(game->maxLockTimer > 1)
        blend += 16-(game->lockTimer * 16) / game->maxLockTimer;

    if(!game->zoneTimer){
        setPawnPalette(11, n, blend);
    }

    if(!game->pawn.big){
        if(game->pawn.y < 16){
            sprite_hide(pawnSprite);
            return;
        }

        sprite_set_attr(pawnSprite, ShapeSquare, 2, 16*7, 11, 2);
        sprite_enable_mosaic(pawnSprite);
        sprite_set_pos(pawnSprite, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->pawn.y - game->boardOffset) * 8 + shake * savefile->settings.shake);
    }else{
        if(game->pawn.y * 2 < 16){
            sprite_hide(pawnSprite);
            return;
        }
        sprite_set_attr(pawnSprite, ShapeSquare, 2, 16*7, 11, 2);
        sprite_enable_mosaic(pawnSprite);
        sprite_enable_affine(pawnSprite, 31, true);
        sprite_set_size(pawnSprite, 1<<7, 31);
        sprite_set_pos(pawnSprite, (10 + game->pawn.x*2) * 8 + push * savefile->settings.shake, (game->pawn.y - game->boardOffset) * 8 * 2 + shake * savefile->settings.shake);
    }
}

void showShadow() {
    pawnShadow = &obj_buffer[113];
    // if (game->clearLock || game->activePiece == -1 || game->gameMode == CLASSIC || (game->gameMode == MASTER && game->level >= 100) || (enableBot && game->gameMode != BATTLE)) {
    if (game->clearLock || game->activePiece == -1 || game->gameMode == CLASSIC || (game->gameMode == MASTER && game->level >= 100)) {
        sprite_hide(pawnShadow);
        return;
    }

    #ifdef PC
    if(flashTimer){
        sprite_hide(pawnShadow);
        return;
    }
    #endif

    u8* shadowTexture;

    bool bld = false;
    switch (savefile->settings.shadow) {
        case 0: shadowTexture = (u8*)sprite2tiles_bin; break;
        case 1: shadowTexture = (u8*)sprite15tiles_bin; break;
        case 2: shadowTexture = (u8*)sprite16tiles_bin; break;
        case 3:
            if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                shadowTexture = blockSprite;
            else
                shadowTexture = (u8*)classicTiles[savefile->settings.skin-7][game->activePiece];
            bld = true;
            break;
        case 4: sprite_hide(pawnShadow); return;
        default: shadowTexture = (u8*)sprite2tiles_bin; break;
    }

    sprite_unhide(pawnShadow, 0);

    int * b = BlockEngine::getShape(game->activePiece, game->pawn.rotation, game->rotationSystem);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(b[i * 4 + j] > 0){
                setSpriteTile(16 * 8, j, i, 4, shadowTexture);
            } else
                clearSpriteTile(16*8, j, i, 4);
        }
    }

    delete[] b;

    int n = game->activePiece;

    if(!game->zoneTimer){
        setPawnPalette(10, n, 14*bld);
    }

    if(!game->pawn.big){
        if(game->pawn.y < 16){
            sprite_hide(pawnShadow);
            return;
        }

        sprite_set_attr(pawnShadow, ShapeSquare, 2, 16 * (8 - (savefile->settings.shadow == 3)), 10, 2);
        sprite_set_pos(pawnShadow, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->lowest() - game->boardOffset) * 8 + shake * savefile->settings.shake);
    }else{
        if(game->pawn.y * 2 < 16){
            sprite_hide(pawnShadow);
            return;
        }
        sprite_set_attr(pawnShadow, ShapeSquare, 2, 16 * (8 - (savefile->settings.shadow == 3)), 10, 2);
        sprite_enable_mosaic(pawnShadow);
        sprite_enable_affine(pawnShadow,30,true);
        sprite_set_size(pawnShadow, 1<<7, 30);

        sprite_set_pos(pawnShadow, (10 + game->pawn.x*2) * 8 + push * savefile->settings.shake, (game->lowest() - game->boardOffset) * 2 * 8 + shake * savefile->settings.shake);
    }
}

void showHold() {
    holdSprite = &obj_buffer[114];
    holdFrameSprite = &obj_buffer[115];
    OBJ_ATTR *background = &obj_buffer[116];

    if(game->gameMode != CLASSIC && !game->zoneTimer){
        sprite_unhide(holdFrameSprite, 0);
        sprite_set_attr(holdFrameSprite,ShapeSquare,2,512,8,3);
        sprite_set_pos(holdFrameSprite, 4 * 8 + 7 + (push < 0) * push, 9 * 8 - 2);

        sprite_unhide(background,0);
        sprite_set_attr(background, ShapeSquare, 2, 512 + 400, 8, 3);
        sprite_enable_blend(background);
        sprite_set_pos(background, 4 * 8 + 7 + (push < 0) * push, 9 * 8 - 2);
    }else{
        sprite_hide(holdFrameSprite);
        sprite_hide(background);
    }

    if (game->held == -1) {
        sprite_hide(holdSprite);
        return;
    }

    #ifdef PC
    if(flashTimer){
        sprite_hide(holdSprite);
        return;
    }
    #endif

    int add = !(game->held == 0 || game->held == 3);
    int palette = (game->canHold)? game->held : 7;

    const int skin = savefile->settings.skin;
    int yoffset = - (6 * (game->rotationSystem != SRS && (skin >= 7 && !(skin == 9 || skin == 10))));

    if (skin < 7 || skin == 9 || skin == 10) {
        sprite_unhide(holdSprite, 0);
        sprite_set_attr(holdSprite, ShapeWide, 2, 9 * 16 + 8 * game->held, palette * (game->zoneTimer == 0), 3);
        sprite_set_pos(holdSprite, (5) * 8 + add * 3 + 3 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0 && game->rotationSystem != ARS));
    } else {
        sprite_unhide(holdSprite, ATTR0_AFF);
        sprite_set_attr(holdSprite, ShapeSquare, 2, 16 * game->held, palette * (game->zoneTimer == 0), 3);
        sprite_enable_affine(holdSprite, 5, false);

        FIXED size;
        if(skin < 9)
            size = 357;//~1.4
        else
            size = 349;//~1.4

        sprite_set_size(holdSprite, size,5);
        sprite_set_pos(holdSprite, (5) * 8 + add * 3 + 3 - 4 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0 && game->rotationSystem != ARS) - 3 + yoffset);
    }
}

void showQueue() {
    int maxQueue = savefile->settings.maxQueue;


    if(game->gameMode == CLASSIC)
        maxQueue = 1;
    else if(game->gameMode == MASTER)
        maxQueue = (maxQueue < 3)? maxQueue : 3;

    for (int i = 0; i < 5; i++)
        queueSprites[i] = &obj_buffer[117 + i];

    for (int i = 0; i < 3; i++)
        queueFrameSprites[i] = &obj_buffer[122 + i];

    OBJ_ATTR *background[3];
    for(int i = 0; i < 3; i++)
        background[i] = &obj_buffer[125+i];

    if(game->zoneTimer){
        for(int i = 0; i < 3; i++){
            sprite_hide(queueFrameSprites[i]);
            sprite_hide(background[i]);
        }
    }else if(maxQueue > 1){
        for (int i = 0; i < 3; i++) {
            sprite_unhide(queueFrameSprites[i], 0);
            sprite_set_attr(queueFrameSprites[i], ShapeSquare, 2, 512 + 16 + 16 * i, 8, 3);
            sprite_set_pos(queueFrameSprites[i], 173 + (push > 0) * push, 12 + 32 * i - (i * 9 * (5-maxQueue)));

            sprite_unhide(background[i], 0);
            sprite_set_attr(background[i], ShapeSquare, 2, 512 + 416 + i * 16, 8, 3);
            sprite_enable_blend(background[i]);
            sprite_set_pos(background[i], 173 + (push > 0) * push, 12 + 32 * i);
        }
    }else{
        sprite_unhide(queueFrameSprites[0], 0);
        sprite_set_attr(queueFrameSprites[0], ShapeSquare, 2, 512, 8, 3);
        sprite_enable_mosaic(queueFrameSprites[0]);
        sprite_set_pos(queueFrameSprites[0], 173 + (push > 0) * push, 12);

        sprite_unhide(background[0], 0);
        sprite_set_attr(background[0], ShapeSquare, 2, 512 + 416, 8, 3);
        sprite_enable_blend(background[0]);
        sprite_set_pos(background[0], 173 + (push > 0) * push, 12);

        for(int i = 1; i < 3; i++){
            sprite_hide(queueFrameSprites[i]);
            sprite_hide(background[i]);
        }
    }

    #ifdef PC
    if(flashTimer){
        for(int i = 0; i < 5; i++)
            sprite_hide(queueSprites[i]);
        return;
    }
    #endif

    int startX = 22 * 8 + 1;
    const int skin = savefile->settings.skin;
    int yoffset = 4 * (maxQueue == 1) - (5 * (game->rotationSystem != SRS && (skin >= 7 && !(skin == 9 || skin == 10))));

    for (int k = 0; k < 5; k++){
        if(k >= maxQueue || k >= game->queue.size()){
            sprite_hide(queueSprites[k]);
            continue;
        }

        int n = game->queue.array[k];

        int add = !(n == 0 || n == 3);
        if (skin < 7 || skin == 9 || skin == 10) {
            sprite_unhide(queueSprites[k], 0);
            sprite_set_attr(queueSprites[k], ShapeWide, 2, 16 * 9 + 8 * n, n * (game->zoneTimer == 0), 3);
            sprite_enable_mosaic(queueSprites[k]);
            sprite_set_pos(queueSprites[k], startX + add * 3 + (push > 0) * push, (3 + (k * 3)) * 6 - 3 * (n == 0) + yoffset + 8 * (game->rotationSystem == ARS && n == 0));
        } else {
            sprite_unhide(queueSprites[k], ATTR0_AFF);
            sprite_set_attr(queueSprites[k], ShapeSquare, 2, 16 * n, n * (game->zoneTimer == 0), 3);
            sprite_enable_affine(queueSprites[k], k, false);
            FIXED size;
            if(skin < 9)
                size = 357;//~1.4
            else
                size = 349;//~1.4

            sprite_set_size(queueSprites[k], size, k);

            sprite_set_pos(queueSprites[k], startX + add * 3 + (push > 0) * push - 4, (3 + (k * 3)) * 6 - 3 * (game->rotationSystem != ARS && n == 0) - 4 + yoffset);
        }
    }
}

bool GameScene::control() {
    if (paused)
        return false;

    key_poll();

    if(demo && key_hit(KEY_FULL)){
        exitGame = true;
        demo = false;
        return true;
    }

    Keys k = savefile->settings.keys;

    if (key_hit(KEY_START) && !multiplayer && !eventPauseTimer && !flashTimer) {
        sfx(SFX_MENUCONFIRM);
        paused = true;
        pauseSong();
        clearText();
        updateText();
    }

    if(replaying){
        replayControl();
        return false;
    }

    if (key_hit(k.hold) && !checkDiagonal(k.hold)) {
        game->hold(1);
    }

    if (key_released(k.hold)){
        game->hold(0);
    }

    if (key_hit(k.moveLeft) && !checkDiagonal(k.moveLeft))
        game->keyLeft(1);
    if (key_hit(k.moveRight) && !checkDiagonal(k.moveRight))
        game->keyRight(1);

    if (key_hit(k.hardDrop) && !checkDiagonal(k.hardDrop))
        game->keyDrop(1);

    if (key_hit(k.softDrop) && !checkDiagonal(k.softDrop))
        game->keyDown(1);

    if (key_is_down(KEY_A) && key_is_down(KEY_B) && savefile->settings.abHold){
        game->hold(1);
    }else{
        if (key_hit(k.rotateCW) && !checkDiagonal(k.rotateCW))
            game->rotateCW(1);

        if (key_hit(k.rotateCCW) && !checkDiagonal(k.rotateCCW))
            game->rotateCCW(1);
    }

    if (key_hit(k.rotate180) && !checkDiagonal(k.rotate180)){
        game->rotateTwice(1);
    }

    if(savefile->settings.abHold && (key_released(KEY_A) || key_released(KEY_B))){
        game->hold(0);
    }

    if (key_released(k.rotateCW))
        game->rotateCW(0);

    if (key_released(k.rotateCCW))
        game->rotateCCW(0);

    if (key_released(k.rotate180)){
        game->rotateTwice(0);
    }

    if (key_released(k.hardDrop))
        game->keyDrop(0);

    if (key_released(k.softDrop))
        game->keyDown(0);

    if (key_released(k.moveLeft)){
        game->keyLeft(0);
    }

    if (key_released(k.moveRight)){
        game->keyRight(0);
    }

    if (key_is_down(KEY_L) && key_is_down(KEY_R) && !(game->gameMode == BATTLE || game->gameMode == MASTER) && !eventPauseTimer) {
        if(!savefile->settings.resetHoldToggle && (restartTimer++ > maxRestartTimer || !savefile->settings.resetHoldType)){
            playAgain = true;
        }
    }else{
        restartTimer = 0;
    }

    if(key_hit(k.zone) && game->gameMode == MARATHON && game->subMode && !game->zoneTimer){
        game->activateZone(1);
    }

    if(savefile->settings.diagonalType == 1 || game->rotationSystem == ARS){
        if(key_released(KEY_RIGHT) || key_released(KEY_LEFT)){
            if(key_is_down(KEY_UP)){
                Function f = getActionFromKey(KEY_UP);
                (game->*f.gameFunction)(1);
            }else if(key_is_down(KEY_DOWN)){
                Function f = getActionFromKey(KEY_DOWN);
                (game->*f.gameFunction)(1);
            }
        }
    }

    if(key_hit(KEY_SELECT)){
        // Values v = evaluate(game->bitboard, game->columnHeights, game->lengthX, game->lengthY, game->stackHeight);
        // int clears = countClears(game->bitboard, game->lengthX, 40, game->stackHeight);

        Weights weights{
        1,
        -4,
        0,
        -1,
        0,
        0,
        -1,
        -1,
        8,
        };

        // int score = (clears * clears) * weights.clears + v.holes * (weights.holes) + (game->lengthY-game->lowest()) * weights.lowest +
        //     v.wells * (weights.well) + v.rowTrans * (weights.row) + v.colTrans * (weights.col) + v.quadWell * (weights.quadWell);

        // log(score);
        std::list<BlockEngine::Move> moves;
        findBestDrop(0, game->pawn.rotation, game, moves, weights);

        log(moves.front().score);
    }

    return false;
}

void GameScene::showTimer() {
    if (!(game->gameMode == TRAINING)) {

        std::string timer = timeToString(gameSeconds);
        aprintClearArea(0, 1, 10, 1);
        aprint(timer, 1 - (timer.size() > 8), 1);
        // wordSprites[11].setText(timer);
        // wordSprites[11].show(8, 8,15);
    }

    if(game->trainingMode){

        // aprint("Finesse", 1, 14);
        wordSprites[4].setText("Finesse");
        wordSprites[4].show(2 * 8, 14 * 8,15);

        // aprintClearArea(0, 15, 10, 15);
        // aprintf(game->finesseFaults, 4, 15);
        wordSprites[5].setTextNum(game->finesseFaults);
        wordSprites[5].show(5 * 8, 15 * 8,15);
    }

    if(proMode){
        // clearSmallText();
        showPPS();
    }
}

void GameScene::showText() {
    if (game->gameMode == MARATHON || game->gameMode == ULTRA || game->gameMode == BLITZ || game->gameMode == CLASSIC) {

        // aprint("Score", 3, 3);
        wordSprites[0].setText("Score");
        wordSprites[0].show(9 * 8 - wordSprites[0].width, 3 * 8,15);

        // std::string score = std::to_string(game->score);
        // int x = 8 - score.size();

        wordSprites[1].setTextNum(game->score);
        wordSprites[1].show(9 * 8 - wordSprites[1].width, 5 * 8,15);

        if (game->gameMode != ULTRA) {
            wordSprites[2].setText("Level");
            wordSprites[2].show(9 * 8 - wordSprites[2].width, 14 * 8,15);

            wordSprites[3].setTextNum(game->level);
            wordSprites[3].show(9 * 8 - wordSprites[3].width, 15 * 8,15);
        }

    } else if (game->gameMode == TRAINING) {
        // aprint("Training", 1, 1);
        wordSprites[0].setText("Training");
        wordSprites[0].show(1 * 8, 1 * 8,15);
    } else if (game->gameMode == DIG && game->subMode) {
        // aprint("Pieces", 2, 14);
        wordSprites[0].setText("Pieces");
        wordSprites[0].show(9 * 8 - wordSprites[0].width, 14 * 8,15);

        // aprintClearArea(0, 15, 10, 1);
        // aprintf(game->pieceCounter, 4, 15);
        wordSprites[1].setTextNum(game->pieceCounter);
        wordSprites[1].show(9 * 8 - wordSprites[1].width, 15 * 8,15);
    } else if (game->gameMode == MASTER){
        // aprint("Grade",3,3);
        wordSprites[0].setText("Grade");
        wordSprites[0].show(9 * 8 - wordSprites[0].width, 3 * 8,15);

        std::string str = GameInfo::masterGrades[game->grade + game->coolCount];

        // aprintClearArea(0, 5, 10, 1);
        // aprint(str,4 - (str.length() == 1),5);
        wordSprites[1].setText(str);
        wordSprites[1].show(9 * 8 - wordSprites[1].width, 5 * 8,15);

        // aprint("Level", 3, 14);
        wordSprites[2].setText("Level");
        wordSprites[2].show(4 * 8, 14 * 8,15);

        str = std::to_string(game->level);
        // aprint(str, 7-str.size(), 16);
        wordSprites[3].setTextNum(game->level);
        wordSprites[3].show((7-str.size()) * 8, 16 * 8,15);

        showSpeedMeter((int)game->speed);

        int n = ((game->level / 100)+1) * 100;

        if(n == 1000)
            n--;

        // aprintf(n, 4, 18);
        wordSprites[4].setTextNum(n);
        wordSprites[4].show(4 * 8, 18 * 8,15);
    }

    if (game->gameMode != BATTLE && game->gameMode != BLITZ && !(game->gameMode == SPRINT && game->subMode == 1) && game->gameMode != MASTER){
        // aprint("Lines", 2, 17);
        // aprintClearArea(0, 18, 10, 1);
        // if (game->gameMode == DIG)
        //     aprintf(game->garbageCleared, 4, 18);
        // else
        //     aprintf(game->linesCleared, 4, 18);

        wordSprites[6].setText("Lines");
        wordSprites[6].show(9 * 8 - wordSprites[6].width, 17 * 8,15);

        int n = 0;
        if (game->gameMode == DIG)
            n = game->garbageCleared;
        else
            n = game->linesCleared;

        wordSprites[7].setTextNum(n);
        wordSprites[7].show(9 * 8 - wordSprites[7].width, 18 * 8,15);

    } else if(game->gameMode == BLITZ){
        // aprint("Lines", 2, 17);
        wordSprites[6].setText("Lines");
        wordSprites[6].show(9 * 8 - wordSprites[6].width, 17 * 8,15);
        std::string str;

        if(game->level > 1)
            str+= std::to_string(game->linesCleared-GameInfo::blitzLevels[game->level-2]);
        else
            str+= std::to_string(game->linesCleared);

        str+= " / ";

        if(game->level > 1)
            str+= std::to_string(GameInfo::blitzLevels[game->level-1] - GameInfo::blitzLevels[game->level-2]);
        else
            str+= "3";

        // aprintClearArea(0, 18, 10, 1);
        // aprint(str,2,18);
        wordSprites[7].setText(str);
        wordSprites[7].show(9 * 8 - wordSprites[7].width, 18 * 8,15);

    } else if(game->gameMode == BATTLE || (game->gameMode == SPRINT && game->subMode == 1)){
        // aprint("Attack", 2, 17);
        // aprintClearArea(0, 18, 10, 1);
        wordSprites[6].setText("Attack");
        wordSprites[6].show(9 * 8 - wordSprites[6].width, 17 * 8,15);

        // aprintf(game->linesSent, 4, 18);
        wordSprites[7].setTextNum(game->linesSent);
        wordSprites[7].show(9 * 8 - wordSprites[7].width, 18 * 8,15);
    }
}

void showPPS(){
    FIXED t = (gameSeconds + game->eventTimer) * float2fx(0.0167f);

    FIXED pps;

    if(t <= 0){
        pps = 0;
    }else{
        pps = fxdiv(int2fx(game->pieceCounter),(t));
    }

    std::string str = "";

    str += std::to_string(fx2int(pps)) + ".";

    int fractional = pps & 0xff;
    for(int i = 0; i < 2; i++){
        fractional *= 10;
        str += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

    clearTiles(2, 113, 3);
    // memset32(&tile_mem[2][113],0,8*3);
    aprints(str,25,0,2);

    aprints("PPS:",0,0,2);
}

void GameScene::showComboStreak(){
    if(game->gameMode != CLASSIC && !game->zoneTimer){
        if (game->comboCounter > 1) {
            // aprint("Combo x", 21, clearTextHeight - 1);
            wordSprites[8].setText("Combo");
            wordSprites[8].show(21 * 8 + 4, (clearTextHeight - 1) * 8,15);

            // aprintf(game->comboCounter, 28, clearTextHeight - 1);
            wordSprites[9].setText("x" + std::to_string(game->comboCounter));
            wordSprites[9].show(21 * 8 + 4, (clearTextHeight) * 8,15);
        } else {
            // aprintClearArea(20, clearTextHeight - 1, 10, 1);
            wordSprites[8].hide();
            wordSprites[9].hide();
        }

        if (game->b2bCounter > 0) {
            // aprint("Streak", 22, clearTextHeight + 1);
            wordSprites[10].setText("Streak");
            wordSprites[10].show(21 * 8 + 4, (clearTextHeight + 2) * 8,15);

            // aprint("x", 24, clearTextHeight + 2);
            // aprintf(game->b2bCounter + 1, 25, clearTextHeight + 2);
            wordSprites[11].setText("x" + std::to_string(game->b2bCounter + 1));
            wordSprites[11].show(21 * 8 + 4, (clearTextHeight + 3) * 8,15);
        } else {
            // aprintClearArea(20, clearTextHeight + 1, 10, 2);
            wordSprites[10].hide();
            wordSprites[11].hide();
        }
    }
}

void showClearText() {

    if(game->zoneTimer && !game->lost)
        return;

    auto index = floatingList.begin();
    while (index != floatingList.end()) {
        std::string text = index->text;

        if (++index->timer > maxClearTextTimer) {
            index = floatingList.erase(index);
            aprintClearArea(9, 0, 12, 1);
        } else {
            int height = 0;
            if (index->timer < 2 * maxClearTextTimer / 3)
                height = 5 * (float)index->timer / ((float)2 * maxClearTextTimer / 3);
            else
                height = (30 * (float)(index->timer) / maxClearTextTimer) - 15;

            if (text.size() <= 10) {
                aprint(text, 15 - text.size() / 2, 15 - height);

            } else {
                std::size_t pos = text.find(" ");

                if(pos != std::string::npos){
                    // aprint("            ", 9, 15 - height);
                    aprintClearArea(9, 15 - height, 12, 1);
                    std::string part1 = text.substr(0, pos);
                    std::string part2 = text.substr(pos + 1);

                    if (15 - height - 1 > 0)
                        aprint(part1, 15 - part1.size() / 2, 15 - height - 1);
                    aprint(part2, 15 - part2.size() / 2, 15 - height);
                }else{
                    aprint(text, 15 - text.size() / 2, 15 - height);
                }
            }
            aprintClearArea(9, 15 - height + 1, 12, 1);
            ++index;
        }
    }
}

void gameLoop(){
    changeScene(new GameScene(),Transitions::FADE);

    exitGame = false;
}

void addGlow(BlockEngine::Drop location) {
    for (int i = 0; i < location.endY && i < 20; i++){
        for (int j = location.startX; j < location.endX; j++){
            glow[i][j] = glowDuration;
        }
    }

    if (game->comboCounter > 0 || game->zoneTimer) {
        int xCenter = (location.endX - location.startX) / 2 + location.startX;
        if (game->previousClear.isTSpin) {
            for (int i = 0; i < 20; i++)
                for (int j = 0; j < 10; j++)
                    glow[i][j] = glowDuration + abs(xCenter - j) + abs(location.endY - i);
        } else {
            for (int i = 0; i < 20; i++)
                for (int j = 0; j < 10; j++)
                    glow[i][j] = glowDuration + Sqrt(abs(xCenter - j) * abs(xCenter - j) + abs(location.endY - i) * abs(location.endY - i));
        }

        if (game->previousClear.isBackToBack == 1)
            effectList.push_back(Effect(1 + (game->previousClear.isTSpin != 0), xCenter, location.endY));

        for (int i = location.startY; i < location.endY && i < 20; i++)
            for (int j = 0; j < 10; j++)
                current[i+1][j+1] = 2048;

        damp = 4;

        dampTimer = dampTimerMax;

    }else{
        for (int i = location.startY; i < location.endY && i < 20; i++)
            for (int j = location.startX; j < location.endX; j++)
                current[i+1][j+1] = 1024;
    }
}

void clearGlow() {
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            glow[i][j] = 0;
    drawGrid();
}

void GameScene::countdown() {
    int timer = 0;
    int timerMax = 120;
    showQueue();
    showHold();
    showPawn();
    showShadow();
    showSprites(128);
    gradient(1);
    showBackground(0);

    paused = true;

    while (closed() && timer++ < timerMax - 1) {
        canDraw = true;
        vsync();
        key_poll();

        if (timer < timerMax / 3) {
            aprint("READY?", 12, 10);
            if (timer == 1 && savefile->settings.announcer){
                sfx(SFX_READY);
            }
        } else if (timer < 2 * timerMax / 3){
            aprint("READY?", 12, 10);
            if(key_is_down(KEY_L) && key_is_down(KEY_R)){
                playAgain = true;
                break;
            }
        }else {
            aprint("  GO  ", 12, 10);
            if (timer == 2 * timerMax / 3 && savefile->settings.announcer)
                sfx(SFX_GO);
        }
    }

    paused = false;

    clearText();
    updateText();

    game->update();

    Keys k = savefile->settings.keys;

    if(key_is_down(k.hold))
        game->hold(1);
    if(key_is_down(k.rotateCW))
        game->rotateCW(1);
    if(key_is_down(k.rotateCCW))
        game->rotateCCW(1);
    if(key_is_down(k.rotate180))
        game->rotateTwice(1);
    if(key_is_down(k.hardDrop))
        game->keyDrop(1);

    if(key_is_down(k.moveLeft)){
        game->keyLeft(1);
    }else if(key_is_down(k.moveRight)){
        game->keyRight(1);
    }

    game->chargeDas();

    if (!(game->gameMode == TRAINING)) {
        playSongRandom(1);
    }

    counted = true;
}

void screenShake() {
    if (!savefile->settings.shake)
        return;

    // int scroll = 0;
    // if(clearTimer){
    //     int n = game->previousClear.linesCleared;

    //     FIXED s = fxdiv(int2fx(8 * n * clearTimer),int2fx(maxClearTimer));

    //     scroll = fx2int(s);
    // }

    setLayerScroll(0, -push, -(shake));
    setLayerScroll(1, -push, -(shake));
    setLayerScroll(3, -push, -(shake));

    // REG_BG0VOFS = -shake;

    // REG_BG1VOFS = -shake;

    #ifdef GBA
    if (shake) {
        if (shake > 0)
            shake--;
        else
            shake++;
        shake *= -1;
    }
    #endif

    #ifdef PC
    shake = fx2int(shakeBuff);
    const FIXED deccel = float2fx(0.768);
    if(shakeBuff){

        shakeBuff = fxmul(shakeBuff,deccel);
        // shake *= deccel;
        // shake = fx2int(shake*deccel);

        if(abs(shakeBuff) < int2fx(1))
            shakeBuff = 0;
    }

    if(shakeVelocity){
        shakeBuff += shakeVelocity;
        shakeVelocity = fxmul(shakeVelocity, deccel);
        if(shakeVelocity <= 0){
            shakeVelocity = 0;
        }

        if(shake >= (shakeMax * (savefile->settings.shakeAmount) / 4)){
            shake = (shakeMax * (savefile->settings.shakeAmount) / 4);
        }
    }

    #endif

    if (game->pushDir != 0) {
        if (abs(push) < pushMax * (savefile->settings.shakeAmount)/4)
            push += game->pushDir * (1 + (savefile->settings.shakeAmount > 2));
    } else {
        if (push > 0)
            push--;
        else if (push < 0)
            push++;
    }
}

void drawFrame(int layer) {
    int l = 25 + layer;

    // u16* dest = (u16*)se_mem[26];

    // dest += 9;

    // int color = 8 * 0x1000;
    int p = 8;
    if(game->zoneTimer)
        p = 0;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 2; j++) {
            setTile(l, j * 11 + 9, i, tileBuild(0x4,(j > 0), false, p));
            // *dest++ = 0x0004 + j * 0x400 + color;
            // dest += 10;
        }
        // dest += 32 - 22;
    }

    #ifdef PC

    if(!game->zoneTimer){
        setTile(l,20,20,tileBuild(29,true,true,p));// right bottom frame corner
        setTile(l,9,20,tileBuild(29,false,true,p));// left bottom frame corner

        for(int i = 0; i < 10; i++)
            setTile(l,10+i,20,tileBuild(28,false,true,p));
    }

    #endif

    progressBar(layer);

    drawGrid();
}

const u8 queueBGLengths[5] = {32,42,60,78,96};

void drawFrameBackgrounds(){
    int maxQueue = savefile->settings.maxQueue;

    if(game->gameMode == MASTER)
        maxQueue = (maxQueue < 3)? maxQueue : 3;

    const int l = queueBGLengths[maxQueue-1];

    TILE *t;

    u8 * src;

    int n = savefile->settings.frameBackground;

    switch(n){
        case 0: src = (u8 *) sprite44tiles_bin; break;
        case 1: src = (u8 *) sprite43tiles_bin; break;
        case 2:
            for(int i = 0; i < 4; i++)
                clearSpriteTiles(512 + 400 + i * 16, 4, 4);
            return;
        default: src = (u8 *) sprite43tiles_bin;
    }

    //draw hold background
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            if(i == 0 && j == 0){
                t = (TILE *) &src[32];
            }else if(i == 0 && j == 3){
                t = (TILE *) &src[64];
            }else if(i == 3 && j == 0){
                t = (TILE *) &src[96];
            }else if(i == 3 && j == 3){
                t = (TILE *) &src[128];
            }else{
                t = (TILE *) src;
            }

            loadSpriteTilesPartial(512 + 400, t, j, i, 1, 1, 4);
        }
    }

    for(int i = 0; i < 3; i++)
        clearSpriteTiles(512 + 416 + i * 16, 4, 4);

    int segments = l / 8;
    int pixelRows = l % 8 - 2;

    if(pixelRows < 0){
        pixelRows = 0;
    }

    //draw queue background
    for(int i = 0; i < segments; i++){
        for(int j = 0; j < 4; j++){
            if(i == 0 && j == 0){
                t = (TILE *) &src[32];
            }else if(i == 0 && j == 3){
                t = (TILE *) &src[64];
            }else if((i == 3 || i == 11) && j == 0){
                t = (TILE *) &src[96];
            }else if((i == 3 || i == 11) && j == 3){
                t = (TILE *) &src[128];
            }else{
                t = (TILE *) src;
            }

            loadSpriteTilesPartial(512 + 416 + 16 * ((int) i/4), t, j, i % 4, 1, 1, 4);
        }
    }

    TILE *temp = new TILE();
    u32 * c = (u32 *) src;

    for(int i = 0; i < 8; i++){
        temp->data[i] = (*c++) * (i < pixelRows);
    }

    for(int i = 0; i < 4; i++)
        loadSpriteTilesPartial(512 + 416 + 16 * ((int) segments/4), temp, i, segments % 4, 1, 1, 4);

    delete temp;
}

INLINE int prep(int n){
    return clamp((n + 500) / 8, 0, 255) / 16;
}

IWRAM_CODE void updateGrid(){
    for(int i = 1; i < 21; i++){
        for(int j = 1; j < 11; j++){
            current[i][j] = (
                previous[i-1][j] + previous[i+1][j] +
                previous[i][j-1] + previous[i][j+1]) / 2 - current[i][j];

            current[i][j] -= current[i][j] >> damp;
        }
    }
}

void drawGrid() {
    std::list<Effect>::iterator index = effectList.begin();

    for (int i = 0; i < (int)effectList.size() && (savefile->settings.effects); i++) {
        if (index->timer < index->duration) {
            switch (index->type) {
            case 0:
                if (index->timer % glowDuration == 0) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            glow[i][j] = glowDuration;
                }
                break;
            case 1:
                if (index->timer == index->duration - 1) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            if (glow[i][j] < glowDuration)
                                glow[i][j] = glowDuration + Sqrt(abs(index->x - j) * abs(index->x - j) + abs(index->y - i) * abs(index->y - i));
                }
                break;
            case 2:
                if (index->timer == index->duration - 1) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            if (glow[i][j] < glowDuration)
                                glow[i][j] = glowDuration + abs(index->x - j) + abs(index->y - i);
                }
                break;
            case 3:
                if (index->timer == 0) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            glow[i][j] = glowDuration + abs(index->x - j) + abs(index->y - i);
                }
                break;
            }

            index->timer++;
        } else {
            index = effectList.erase(index);
        }

        index++;
    }

    u32 gridTile;
    int palOffset = 4;

    switch (savefile->settings.backgroundGrid) {
        case 0: gridTile = 0x0002; break;
        case 1: gridTile = 0x000c; break;
        case 2: gridTile = 0x001a; break;
        case 3: gridTile = 0x001e; break;
        case 4: gridTile = 0x001f; break;
        case 5: gridTile = 0x0020; break;
        case 6: gridTile = 0x0024; break;
        case 7: gridTile = 0x0000; break;
        default: gridTile = 0x0002; break;
    }

    #ifdef GBA
    setTiles(26,31*32+10, 10, tileBuild(gridTile, false, false, palOffset * (savefile->settings.lightMode && !game->zoneTimer)));
    setTiles(26,20*32+10, 10, tileBuild(gridTile, false, false, palOffset * (savefile->settings.lightMode && !game->zoneTimer)));
    #endif

    if(savefile->settings.effects < 2){
        u32 t = tileBuild(gridTile, false, false, palOffset * (savefile->settings.lightMode && !game->zoneTimer));

        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 10; j++) {
                if (glow[i][j] == 0 || !savefile->settings.effects) {
                    setTile(26, 10 + j, i, t);
                } else if (glow[i][j] > glowDuration) {
                    glow[i][j]--;
                    setTile(26, 10 + j, i, t);
                } else if (glow[i][j] > 0) {
                    glow[i][j]--;
                    int color = 0;
                    if (glow[i][j] >= glowDuration * 3 / 4)
                        color = 3;
                    else if (glow[i][j] >= glowDuration * 2 / 4)
                        color = 2;
                    else if (glow[i][j] >= glowDuration * 1 / 4)
                        color = 1;
                    setTile(26, 10 + j, i, tileBuild(gridTile, false, false, color + (palOffset) * (savefile->settings.lightMode && !game->zoneTimer)));
                }
            }
        }
    }else if(savefile->settings.effects == 2){
        for(int i = 0; i < 20; i++)
            for(int j = 0; j < 10; j++)
                setTile(26, j + 10, i, tileBuild(gridTile,false,false,prep(current[i+1][j+1])));
    }
}

void progressBar(int layer) {
    if (game == nullptr || game->goal == 0 || game->zoneTimer)
        return;

    int l = 25 + layer;

    int current;
    int max = game->goal;

    if (game->gameMode == DIG)
        current = game->garbageCleared;
    else if (game->gameMode == ULTRA || game->gameMode == BLITZ)
        current = game->timer;
    else if (game->gameMode == SPRINT && game->subMode)
        current = game->linesSent;
    else if (game->gameMode != SURVIVAL)
        current = game->linesCleared;
    else{
        setTile(l,20,20,tileBuild(29,true,true,8));// right bottom frame corner
        return;
    }


    if (game->gameMode != BATTLE) {
        showBar(current, max, 20, 8, layer);
        setTile(l,20,20,tileBuild(33,false,false,8));// right bottom frame corner
        setTile(l,9,20,tileBuild(29,false,true,8));// left bottom frame corner
    }else{
        if (++attackFlashTimer > attackFlashMax)
            attackFlashTimer = 0;

        if (attackFlashTimer < attackFlashMax / 2) {
            setPaletteColor(8, 4, 0x421f, 1);
            // memset32(&pal_bg_mem[8 * 16 + 5], 0x421f, 1);
        } else {
            setPaletteColor(8, 4, 0x7fff, 1);
            // memset32(&pal_bg_mem[8 * 16 + 5], 0x7fff, 1);
        }

        //attack bar
        showBar(game->getIncomingGarbage(), 20, 9, 8, layer);
        setTile(l,9,20,tileBuild(33,true,false,8)); // left bottom frame corner
    }
}

void showBar(int current, int max, int x, int palette, int layer) {
    // #ifdef GBA
    // palette *= 0x1000;

    if (max > 10000) {
        current /= 10;
        max /= 10;
    }

    int l = 25 + layer;

    int pixels = fx2int(fxmul(fxdiv(int2fx(current), int2fx(max)), int2fx(158)));
    int segments = fx2int(fxdiv(int2fx(current), fxdiv(int2fx(max), int2fx(20))));
    int edge = pixels - segments * 8 + 1 * (segments != 0);

    // u16* dest = (u16*)se_mem[26];

    // dest += x;
    for (int i = 0; i < 20; i++) {

        if (i == 0) {
            if (segments == 19) {
                if (edge == 0)
                    setTile(l,x,i,tileBuild(0x6,false,false,palette));
                    // *dest = 0x0006 + palette;
                else {
                    setTile(l,x,i,tileBuild(0xa,false,false,palette));
                    // *dest = 0x000a + palette;
                    for (int j = 0; j < 7; j++) {
                        // TILE* tile = &tile_mem[0][10];
                        if (j == 0) {
                            if (6 - j > edge)
                                setTileRow(0,10,j+1,0x13300332);
                                // tile->data[j + 1] = 0x13300332;
                            else
                                setTileRow(0,10,j+1,0x13344332);
                                // tile->data[j + 1] = 0x13344332;
                        } else {
                            if (6 - j > edge)
                                setTileRow(0,10,j+1,0x13000032);
                                // tile->data[j + 1] = 0x13000032;
                            else
                                setTileRow(0,10,j+1,0x13444432);
                                // tile->data[j + 1] = 0x13444432;
                        }
                    }
                }
            } else if (segments >= 20)
                setTile(l,x,i,tileBuild(0x8,false,false,palette));
                // *dest = 0x0008 + palette;
            else
                setTile(l,x,i,tileBuild(0x6,false,false,palette));
                // *dest = 0x0006 + palette;

        } else if (i == 19) {
            if (segments == 0) {
                if (edge == 0)
                    setTile(l,x,i,tileBuild(0x6,false,true,palette));
                    // *dest = 0x0806 + palette;
                else {
                    // *dest = 0x080a + palette;
                    setTile(l,x,i,tileBuild(0xa,false,true,palette));
                    for (int j = 0; j < 7; j++) {
                        // TILE* tile = &tile_mem[0][10];
                        if (j == 0) {
                            if (j >= edge)
                                setTileRow(0,10,j+1,0x13300332);
                                // tile->data[j + 1] = 0x13300332;
                            else
                                setTileRow(0,10,j+1,0x13344332);
                                // tile->data[j + 1] = 0x13344332;
                        } else {
                            if (j >= edge)
                                setTileRow(0,10,j+1,0x13000032);
                                // tile->data[j + 1] = 0x13000032;
                            else
                                setTileRow(0,10,j+1,0x13444432);
                                // tile->data[j + 1] = 0x13444432;
                        }
                    }
                }
            } else
                setTile(l,x,i,tileBuild(0x8,false,true,palette));
                // *dest = 0x0808 + palette;
        } else {
            if (19 - i > segments) {
                setTile(l,x,i,tileBuild(0x7,false,false,palette));
                // *dest = 0x0007 + palette;
            } else if (19 - i == segments) {
                setTile(l,x,i,tileBuild(0xb,false,false,palette));
                // *dest = 0x000b + palette;
                for (int j = 0; j < 7; j++) {
                    // TILE* tile = &tile_mem[0][11];
                    if (7 - j > edge) {
                        setTileRow(0,11,j+1,0x13000032);
                        // tile->data[j] = 0x13000032;
                    } else {
                        setTileRow(0,11,j+1,0x13444432);
                        // tile->data[j] = 0x13444432;
                    }
                }
            } else {
                setTile(l,x,i,tileBuild(0x9,false,false,palette));
                // *dest = 0x0009 + palette;
            }
        }

        // dest += 32;
    }

    // #endif
}

void showBestMove(){
    List moveList = game->previousBest;

    moveList.pop_back();

    for(int i = 0; i < 3; i++)
        moveSprites[i] = &obj_buffer[16+i];

    for(int i = 0; i < 3; i++){
        if(i > (int) moveList.size() - 1){
            sprite_hide(moveSprites[i]);
            continue;
        }
        sprite_unhide(moveSprites[i],0);

        sprite_set_attr(moveSprites[i], ShapeSquare, 0, 512 + 128 + moveList.array[i], 15, 2);
        // obj_set_attr(moveSprites[i], 0, 0, ATTR2_BUILD(512 + 128 + *move, 15, 2));

        sprite_set_pos(moveSprites[i], 38 + 12 * i, 54);
    }
}

bool checkDiagonal(int key){
    if((!savefile->settings.diagonalType || game->rotationSystem == NRS) && game->rotationSystem != ARS)
        return false;
    return ((key == KEY_DOWN || key == KEY_UP) && (key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)));
}

void showPlaceEffect(){
    #ifdef PC
    return;
    #endif

    const int tileStart = 512 - 32 * 3;

    for(int i = 0; i < 3; i++)
        sprite_hide(&obj_buffer[19+i]);

    auto it = placeEffectList.begin();
    for(int i = 0; it != placeEffectList.end(); i++){
        if(it->timer == 0){
            sprite_hide(it->sprite);
            placeEffectList.erase(it++);
            i--;
            continue;
        }

        bool flip = false;
        int xoffset = 0;
        int yoffset = 0;
        int r = it->rotation;

        int n = 2 - ((it->timer-1)/4);

        switch(it->piece){
        case 0:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n],size/2);
            if(game->rotationSystem == NRS){
                if(r % 2 == 1)
                    r = 1;
                else
                    r = 2;
            }else if(game->rotationSystem == ARS){
                if(r % 2 == 1)
                    r = 1;
                else
                    r = 0;
            }
            break;
        case 1:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n + 3], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+3],size/2);
            xoffset = yoffset = -4;

            if (game->rotationSystem == ARS && r == 2)
                yoffset += 8;

            if(game->rotationSystem == NRS || game->rotationSystem == ARS)
                r = (r+2) % 4;

            break;
        case 2:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n + 3], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+3],size/2);
            xoffset = yoffset = -4;
            flip = true;

            if (game->rotationSystem == ARS && r == 2)
                yoffset += 8;

            if(game->rotationSystem == NRS || game-> rotationSystem == ARS)
                r = (r+2) % 4;
            break;
        case 3:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n + 6], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+6],size/2);
            yoffset = -4 + (game->rotationSystem == NRS || game-> rotationSystem == ARS) * 8;
            r = 0;
            break;
        case 4:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n + 9], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+9],size/2);
            xoffset = -4;
            yoffset = -4;
            if(game->rotationSystem == NRS || game-> rotationSystem == ARS){
                if(r % 2 == 1)
                    r = 1 + 2 * (game->rotationSystem == ARS);
                else
                    r = 2;
            }
            break;
        case 5:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n + 12], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+12],size/2);
            xoffset = yoffset = -4;

            if (game->rotationSystem == ARS && r == 2)
                yoffset += 8;

            if(game->rotationSystem == NRS || game-> rotationSystem == ARS)
                r = (r+2) % 4;
            break;
        case 6:
            loadSpriteTiles(tileStart + 32 * i, placeEffectTiles[n + 9], 8, 3);
            // memcpy32(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+9],size/2);
            xoffset = -4;
            yoffset = -4;
            flip = true;
            if(game->rotationSystem == NRS || game-> rotationSystem == ARS){
                if(r % 2 == 1)
                    r = 1;
                else
                    r = 2;
            }
            break;
        default:
            break;
        }

        if(!flip){
            if((r == 1 || r == 2))
                xoffset += -1;
            if(r > 1)
                yoffset += -1;
        }else{
            if((r == 1 || r == 2))
                yoffset += -1;
            if(r < 2)
                xoffset += -1;
        }

        FIXED spin = 0;
        int x,y;

        if(it->timer > 5 && it->piece != 3){
            spin = 0x4000 * (it->rotating) * ((it->timer-6));
            spin /= 12;

            if(it->dx || it->dy){
                int mix = 255 * (6-(it->timer/2));

                x = fx2int(lerp(int2fx(it->x - (it->dx*2)), int2fx(it->x), mix/6));
                y = fx2int(lerp(int2fx(it->y - (it->dy*2)), int2fx(it->y), mix/6));
            }else{
                x = it->x;
                y = it->y;
            }
        }else{
            x = it->x;
            y = it->y;
        }

        x += xoffset + push * savefile->settings.shake;
        y += yoffset - shake * savefile->settings.shake;

        it->sprite = &obj_buffer[19+i];
        sprite_unhide(it->sprite,ATTR0_AFF_DBL);
        sprite_set_attr(it->sprite, ShapeWide, 3, tileStart + 32 * i, (game->zoneTimer != 0)?11:it->piece, 3 - (it->rotating != 0));
        sprite_enable_affine(it->sprite, 7+i, true);
        sprite_set_pos(it->sprite, x, y);
        sprite_rotscale(it->sprite, ((flip)?-1:1) << 8, 1<<8, - 0x4000 * (r) + spin,7+i);

        it->timer--;
        it++;
    }
}

void addPlaceEffect(BlockEngine::Drop drop){
    if((int)placeEffectList.size() >= 3 || !savefile->settings.placeEffect || game->pawn.big)
        return;

    placeEffectList.push_back(PlaceEffect(drop.x, drop.y, drop.dx, drop.dy, drop.piece, drop.rotation, drop.rotating * (game->gameMode != CLASSIC)));
}

void hideMinos(){
    sprite_hide(pawnSprite);
    sprite_hide(pawnShadow);
    sprite_hide(holdSprite);

    for(int i = 0; i < 5; i++)
        sprite_hide(queueSprites[i]);

    for(int i = 0; i < 3; i++)
        sprite_hide(&obj_buffer[19+i]);

    sprite_hide(&obj_buffer[22]);
}

void showSpeedMeter(int fill){
    //TODO:
    #ifdef GBA
    const int maxLength = 20;

    fill *= 2;

    OBJ_ATTR * sprite = &obj_buffer[22];

    //set palette
    setPaletteColor(16 + 13, 1, 0x03e0, 1);
    setPaletteColor(16 + 13, 2, 0x001f, 1);
    setPaletteColor(16 + 13, 3, 0x4a52, 1);
    // memset16(&pal_obj_mem[13*16+1],0x03e0,1); //green
    // memset16(&pal_obj_mem[13*16+2],0x001f,1); //red
    // memset16(&pal_obj_mem[13*16+3],0x4a52,1); //gray

    //initialize tile with 0s
    TILE *dest;
    for(int x = 0; x < 4; x++){
        dest = (TILE *) &tile_mem[4][256 + x];
        for(int y = 0; y < 8; y++){
            dest->data[y] = 0;
        }
    }

    if(fill > maxLength)
        fill = maxLength;

    //write color c at specific location depending on if x is less than max length
    for(int x = 0; x < maxLength+1; x++){
        dest = (TILE *) &tile_mem[4][256 + x / 8];
        int shift = ((x%8) * 4);
        int c = (1 + (x < fill)) << shift;//color
        if(x < maxLength){
            dest->data[0] |= c;//set color
            if(x > 0)
                dest->data[1] |= 3 << shift;//gray
        }else{
            dest->data[1] |= 3 << shift;//gray
        }
    }

    obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(1), ATTR2_BUILD(256, 13, 0));
    obj_set_pos(sprite,35, 139);
    obj_unhide(sprite, 0);

    #endif
}

void disappear(){
    std::tuple<u8,u8> coords;
    // u16* dest = (u16*)se_mem[25];

    bool found = false;

    while(!game->toDisappear.empty()){
        coords = game->toDisappear.front();

        int x = std::get<0>(coords);
        int y = std::get<1>(coords);

        // clearTiles(25,(y-20)*32 + x + 10,1);
        setTile(25,x+10,y-20,0);

        found = true;

        game->toDisappear.pop_front();
    }

    if(found){
        showBackground(0);
    }
}

INLINE int getBoard(int x, int y){
    if(game->disappearTimers[y][x] == 1)
        return 0;

    return game->board[y][x];
}

void showZoneMeter(){
    if(!(game->gameMode == MARATHON && game->subMode))
        return;

    OBJ_ATTR * sprite = &obj_buffer[23];

    #ifdef PC
    if(flashTimer){
        sprite_hide(sprite);
        return;
    }
    #endif

    sprite_set_attr(sprite, ShapeSquare, 2, 256 + 3, 12, 0);
    sprite_enable_mosaic(sprite);
    sprite_set_pos(sprite, 9, 74);
    sprite_unhide(sprite,0);

    const int color = 0x7fff;
    const int anticolor = 0x0c63;
    const int disabled = 0x5294;

    int n;
    if(!game->zoneTimer){
        int part = game->zoneCharge / 8;
        n = part * 3 + (game->zoneCharge % 8) / 3;
    }else{
        n = game->zoneTimer / 100 + !(game->zoneTimer == 1);
    }

    for(int i = 0; i < 12; i++){
        if(i < n && n <= 2 && !game->zoneTimer)
            setPaletteColor(12 + 16, 4+i, disabled, 1);
        else if (i < n)
            setPaletteColor(12 + 16, 4+i, color, 1);
        else
            setPaletteColor(12 + 16, 4+i, anticolor, 1);
    }

}

void zoneFlash(){
    if(flashTimer == flashTimerMax){
        if(previousPalette != nullptr)
            delete previousPalette;
        previousPalette = new COLOR [512];
        savePalette(previousPalette);
        // memcpy32(&previousPalette[0], pal_bg_mem, 256);
    }

    flashTimer--;

    setMosaic(flashTimer, flashTimer);

    int n = ((float)flashTimer/flashTimerMax) * 31;

    color_fade_fast(16, 0, previousPalette, 0x7fff, 128, n);

    bool cond = ((flashTimer < flashTimerMax/2) && eventPauseTimer);

    if(cond && game->gameMode == MARATHON)
        gradient(true);

    mirrorPalettes(cond,8);

    if(flashTimer < flashTimerMax/2 && game->gameMode == MASTER){
        if(!savefile->settings.lightMode)
            setPaletteColor(0, 0, 0x0000, 1);
        else
            setPaletteColor(0, 0, 0x5ad6, 1);
    }
}

void resetZonePalette(){
    if(!holdingSave)
        return;

    holdingSave = false;

    savefile->settings = previousSettings;
    setPalette();
    setClearEffect();

    setMusicTempo(1024);
    setMusicVolume(512 * ((float)savefile->settings.volume / 10));
    // mmSetModuleTempo(1024);

    loadTiles(0, 4, sprite5tiles_bin, 1);
    // memcpy16(&tile_mem[0][4],sprite5tiles_bin,16);

    if(game->lost)
        return;

    // u16 * dest = (u16*) &se_mem[26];
    for(int i = 0; i < 20; i++){
        setTile(25, 9, i, tileBuild(4, false, false, 8));
        setTile(25, 20, i, tileBuild(4, true, false, 8));
        // dest[i * 32 + 9] = 4 + 0x8000;
        // dest[i * 32 + 20] = 4 + 0x8400;
    }

    #ifdef PC

    setTile(25,20,20,tileBuild(29,true,true,8));// right bottom frame corner
    setTile(25,9,20,tileBuild(29,false,true,8));// left bottom frame corner

    for(int i = 0; i < 10; i++)
        setTile(25,10+i,20,tileBuild(28,false,true,8));

    #endif

    if(!eventPauseTimer)
        flashTimer = 0;
}

void showFinesseCombo(){
    OBJ_ATTR * sprite = &obj_buffer[24];

    if(game->rotationSystem != SRS || game->finesseStreak < 3){
        sprite_hide(sprite);
        return;
    }

    sprite_set_attr(sprite, ShapeWide, 0, 275, 15, 0);
    // obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(0),ATTR2_BUILD(275, 15, 0));
    sprite_set_pos(sprite, 52, 104);
    sprite_unhide(sprite,0);

    clearSpriteTiles(275, 2, 1);
    // memset32(&tile_mem[4][275],0x0000, 8 * 2);

    std::string text = "x" + std::to_string(game->finesseStreak);
    aprintsSprite(text,0,0,275);
}

void frameSnow(int layer){
    const int fillAmount = 20;

    int l = 25 + layer;

    TILE * t = (TILE *) sprite5tiles_bin;

    TILE * dest = new TILE();
    TILE * topTile = new TILE();

    // TILE * dest = (TILE *) &tile_mem[0][4];
    // TILE * topTile = (TILE *) &tile_mem[0][46];

    for(int i = 0; i < 8; i++){
        int add1 = 0;
        int add2 = 0;
        for(int j = 0; j < 4; j++){
            add1 += ((qran () % 100 < fillAmount) * 0xf) << ((j+4)*4);
            add2 += (((qran () % 100) < ((10-i) * 10) ) * 0xf) << ((j+4)*4);
        }

        dest->data[i] = t->data[i] & ~add1;
        topTile->data[i] = t->data[i] & ~add2;
    }

    loadTiles(0, 4, dest, 1);
    loadTiles(0, 46, topTile, 1);

    // u16 * dest2 = (u16 *) &se_mem[25];

    int i;
    for(i = 0; i < 20 - ((20 * game->zoneTimer) / game->zoneStart); i++){
        setTile(l, 9, i, 0);
        setTile(l, 20, i, 0);
        // dest2[i * 32 + 9] = 0;
        // dest2[i * 32 + 20] = 0;
    }

    if(i == 20)
        i = 19;

    setTile(l, 9, i, tileBuild(46,false,false,0));
    setTile(l, 20, i, tileBuild(46,true,false,0));
    // dest2[i * 32 + 9] = 46;
    // dest2[i * 32 + 20] = 46 + 0x400;

    #ifdef PC

    setTile(l,20,20,tileBuild(29,true,true,0));// right bottom frame corner
    setTile(l,9,20,tileBuild(29,false,true,0));// left bottom frame corner

    for(int i = 0; i < 10; i++)
        setTile(l,10+i,20,tileBuild(28,false,true,0));

    #endif

    delete dest;
    delete topTile;

}

void rainbowPalette(){
    int n = (rainbowTimer >> 3) + 24;

    int color = 0;
    if(!savefile->settings.lightMode)
        color = RGB15(31, n, n);
    else
        color = RGB15(n, 31, 31);

    clr_rgbscale((COLOR *) rainbow,(COLOR *)&palette[0][1],5,color);

    if(savefile->settings.lightMode){
        loadPalette(16, 1, rainbow, 1);
        loadPalette(16, 2, rainbow, 4);
        // memcpy16(&pal_obj_mem[1],rainbow,1);
        // memcpy16(&pal_obj_mem[2],rainbow,4);
    }

    if(!savefile->settings.lightMode)
        color_fade((COLOR *) rainbow, (COLOR *) rainbow, 0, 4, 10);
        // clr_fade((COLOR *) rainbow,0,(COLOR*) rainbow,4,10);
    else
        color_adj_MEM((COLOR *) rainbow, (COLOR *) rainbow, 4, float2fx(0.25));
        // clr_adj_brightness((COLOR *) rainbow,(COLOR*) rainbow,4,float2fx(0.25));

    loadPalette(0, 1, rainbow, 4);
    // memcpy16(&pal_bg_mem[1],rainbow,4);

    if(!savefile->settings.lightMode)
        loadPalette(16, 1, rainbow, 4);
        // memcpy16(&pal_obj_mem[1],rainbow,4);

    if(savefile->settings.shadow != 3){
        loadPalette(16 + 10, 1, rainbow, 4);
        // memcpy16(&pal_obj_mem[10*16+1],rainbow,4);
    }else{
        if(!savefile->settings.lightMode)
            color_fade_palette(16 + 10, 1, (COLOR *) rainbow, 0, 4, 14);
        //     clr_fade((COLOR*)rainbow, 0x0000, &pal_obj_mem[10 * 16+1], 4, 14);
        else
            color_adj_brightness(16 + 10, 1, (COLOR *) rainbow, 4, (float2fx(0.25)));
        //     clr_adj_brightness(&pal_obj_mem[10 * 16+1], (COLOR*)rainbow, 4, float2fx(0.25));
    }
    loadPalette(16 + 11, 1, rainbow, 4);
    // memcpy16(&pal_obj_mem[11*16+1],rainbow,4);
}

void showZoneText(){
    if(game->zonedLines == 0)
        return;

    aprintClearArea(10, 0, 10, 20);

    int height = (game->lengthY - game->zonedLines) / 2;

    char buff[4];

    int lines = game->zonedLines / (1 + game->pawn.big);

    posprintf(buff, "%d", lines);

    std::string text = buff;

    text += " line";

    if(lines > 1)
        text+= "s";

    const u16 pal[2][2] = {{0x5294,0x0421},{0x5294,0x318c}};

    if(!savefile->settings.lightMode)
        loadPalette(13, 2, pal[0], 2);
        // memcpy16(&pal_bg_mem[13 * 16 + 2],pal[0],2);
    else
        loadPalette(13, 2, pal[1], 2);
        // memcpy16(&pal_bg_mem[13 * 16 + 2 ],pal[1],2);

    aprintColor(text, 15 - text.size()/2, height , 2);
}

static FIXED creditCurrentHeight = int2fx(SCREEN_HEIGHT);
static s8 creditIndex = 0;
static u8 creditLastRead = 0;

const FIXED creditSpeed = float2fx(0.6);
const int creditSpace = 200;

void setupCredits(){
    creditCurrentHeight = int2fx(SCREEN_HEIGHT);
    creditIndex = 0;
    creditLastRead = 0;

    clearSpriteTiles(256, MAX_WORD_SPRITES * 12 * 8, 1);
    // memset32(&tile_mem[4][256], 0x0000 , MAX_WORD_SPRITES * 12 * 8);
    for (int i = 0; i < MAX_WORD_SPRITES; i++){
        delete wordSprites[i];
        wordSprites[i] = new WordSprite(i,64 + i * 3, 300 + i * 12);
    }

    for(int i = 0; i < 2; i++){
        auto index = GameInfo::credits.begin();
        std::advance(index,creditLastRead++);

        std::string text = *index;
        std::size_t pos = text.find(" ");

        if(pos != std::string::npos){
            // aprint("            ", 9, 15 - height);
            std::string part1 = text.substr(0, pos);
            std::string part2 = text.substr(pos + 1);

            wordSprites[i*3+0]->setText(part1);
            wordSprites[i*3+1]->setText(part2);
        }else{
            wordSprites[i*3+1]->setText(text);
        }

        wordSprites[i*3+2]->setText("akouzoukos");
    }

    for(int i = 0; i < 9; i++){
        wordSprites[i]->priority = 3;
    }
}

void showCredits(){

    creditCurrentHeight -= creditSpeed;

    int height = fx2int(creditCurrentHeight);

    // profile_start();
    for(int i = 0; i < 3; i++){
        int yconst = height + creditSpace * i;
        int id = ((i+creditIndex)%3)*3;
        for(int j = 0; j < 3; j++){
            int y = 8 * (j + (j == 2)) + yconst;
            if(y > SCREEN_HEIGHT || y < -8){
                wordSprites[id + j]->hide();
            }else{
                wordSprites[id + j]->show(10 * 8, y, 15);
            }
        }
    }
    // log(std::to_string(profile_stop()));

    if(fx2int(creditCurrentHeight) <= -32)
        creditRefresh = true;
}

void refreshCredits(){
    creditRefresh = false;

    creditIndex++;

    creditCurrentHeight += int2fx(creditSpace);

    if(creditIndex >= 3)
        creditIndex = 0;

    auto index = GameInfo::credits.begin();
    std::advance(index,creditLastRead);

    std::string text;
    if(creditLastRead >= 10)
        text = "";
    else{
        text = *index;
        creditLastRead++;
    }

    std::size_t pos = text.find(" ");

    if(pos != std::string::npos){
        std::string part1 = text.substr(0, pos);
        std::string part2 = text.substr(pos + 1);


        wordSprites[creditIndex*3+0]->setText(part1);
        wordSprites[creditIndex*3+1]->setText(part2);
    }else{
        wordSprites[creditIndex*3+0]->setText("");
        wordSprites[creditIndex*3+1]->setText(text);
    }

    if(creditLastRead < 11)
        wordSprites[creditIndex*3+2]->setText("akouzoukos");
    else
        wordSprites[creditIndex*3+2]->setText("");

}

void showFullMeter(){
    if(game->zoneCharge != 32){
        if(fullMeterTimer < fullMeterAnimationLength){
            fullMeterTimer = 0;
            loadSpriteTiles(256+3, meterTiles[0], 4, 4);
            // memcpy32(&tile_mem[4][256+3],meterTiles[0],meter1_tiles_bin_size/4);
        }
        return;
    }

    if(--fullMeterTimer <= 0){
        fullMeterTimer = fullMeterTimerMax;
        loadSpriteTiles(256+3, meterTiles[0], 4, 4);
        // memcpy32(&tile_mem[4][256+3],meterTiles[0],meter1_tiles_bin_size/4);
        return;
    }

    if(fullMeterTimer < fullMeterAnimationLength){
    int n = 2 - (fullMeterTimer/(fullMeterAnimationLength/3));
        loadSpriteTiles(256+3, meterTiles[n+1], 4, 4);
        // memcpy32(&tile_mem[4][256+3],meterTiles[n+1],meter1_tiles_bin_size/4);
    }
}

const Function gameFunctions[9] ={
    {&Game::keyLeft},
    {&Game::keyRight},
    {&Game::rotateCW},
    {&Game::rotateCCW},
    {&Game::rotateTwice},
    {&Game::keyDown},
    {&Game::keyDrop},
    {&Game::hold},
    {&Game::activateZone},
};

Function getActionFromKey(int key){
    int *keys = (int *) &savefile->settings.keys;

    int k = 0;
    for(int i = 0; i < 9; i++){
        if(keys[i] == key){
            k = i;
            break;
        }
    }

    return gameFunctions[k];
}

void liftKeys(){
    game->liftKeys();
}

void setRandomGraphics(Save *save){
    save->settings.edges = qran () % 2;
    save->settings.backgroundGrid = qran() % (MAX_BACKGROUNDS - 1);
    save->settings.skin = qran() % (MAX_SKINS - 1);
    save->settings.palette = qran() % 8;
    save->settings.shadow = qran() % (MAX_SHADOWS - 1);
    save->settings.colors = qran() % (MAX_COLORS - 1);
    save->settings.backgroundGradient = (RGB15(qran() % 31, qran() % 31, qran() % 31) << 16) + RGB15(qran() % 31, qran() % 31, qran() % 31);
    save->settings.frameBackground = qran() % (MAX_FRAME_BACKGROUNDS - 1);

    save->settings.backgroundType = ((qran() % 10) > 3);

    if(save->settings.colors == 3){
        save->settings.shadow = 3;
    }

    if(qran() % 10 == 0)
        save->settings.backgroundGradient = 0;
}

void setJourneyGraphics(Save *save, int level){
    switch (level){
        // case 1:
        //     save->settings.backgroundGradient = 0x7dc8;
        //     save->settings.backgroundGrid = 5;
        //     save->settings.skin = 11;
        //     save->settings.shadow = 3;
        //     save->settings.palette = 5;
        //     save->settings.colors = 1;
        //     save->settings.edges = true;
        //     save->settings.lightMode = false;
        //     break;
        // case 2:
        //     save->settings.backgroundGradient = RGB15(0,0,0);
        //     save->settings.backgroundGrid = 0;
        //     save->settings.skin = 0;
        //     save->settings.shadow = 3;
        //     save->settings.palette = 0;
        //     save->settings.colors = 1;
        //     save->settings.edges = false;
        //     save->settings.lightMode = false;
        //     break;
        // case 3:
        //     save->settings.backgroundGradient = RGB15(0,0,0);
        //     save->settings.backgroundGrid = 0;
        //     save->settings.skin = 0;
        //     save->settings.shadow = 3;
        //     save->settings.palette = 0;
        //     save->settings.colors = 4;
        //     save->settings.edges = false;
        //     save->settings.lightMode = false;
        //     break;
        default:
            setRandomGraphics(save);
            break;
    }
}

std::list<Timestamp*>::iterator replayIterator;

void replayControl(){
    Timestamp* t = *replayIterator;

    while(replayIterator != currentReplay.end() && t->timer == game->timer){
        bool dir = t->dir;
        switch(t->move){
        case 0:
            game->keyLeft(dir);
            break;
        case 1:
            game->keyRight(dir);
            break;
        case 2:
            game->rotateCW(dir);
            break;
        case 3:
            game->rotateCCW(dir);
            break;
        case 4:
            game->rotateTwice(dir);
            break;
        case 5:
            game->hold(dir);
            break;
        case 6:
            game->keyDown(dir);
            break;
        case 7:
            game->keyDrop(dir);
            break;
        }

        ++replayIterator;
        t = *replayIterator;
    }
}

void GameScene::init(){
    reset();
    loadSpriteTiles(512, sprite6tiles_bin,4,4);

    loadSpriteTiles(512 + 16, sprite6tiles_bin,4,3);
    loadSpriteTilesPartial(512 + 16, &sprite6tiles_bin[128], 0, 3,4,1,4);
    loadSpriteTiles(512 + 32, &sprite6tiles_bin[128],4,2);
    loadSpriteTilesPartial(512 + 32, &sprite6tiles_bin[128],0,2,4,2,4);
    loadSpriteTiles(512 + 48, &sprite6tiles_bin[128],4,1);
    loadSpriteTilesPartial(512 + 48, &sprite6tiles_bin[128],0,1,4,3,4);

    setGradient(savefile->settings.backgroundGradient);

    setSkin();

    clearTilemap(25);

    clearTiles(2,110,400);
    clearSpriteTiles((927 - 32 * 3), 8 * 5, 3); // clear tiles in place Effect area
    setSmallTextArea(110, 3, 7, 9, 10);
    clearText();
    gameSeconds = 0;

    loadSpriteTiles(256+3, meterTiles[0], 4, 4);

    for(int i = 0; i < 3; i++)
        moveSprites[i] = &obj_buffer[16+i];

    for(int i = 0; i < MAX_WORD_SPRITES; i++){
        wordSprites[i].setup(i, 40 + i * 3, 672 + i * 12,false);
    }

    clearSprites(128);
    updateText();

    drawFrame(0);
    drawFrameBackgrounds();

    stopSong();

    showHold();

    showQueue();

    if(game->gameMode == MASTER)
        showSpeedMeter((int)game->speed);

    showSprites(128);

    if(game->gameMode == MASTER){
        maxClearTimer = game->maxClearDelay;
    }else if(proMode && game->rotationSystem == SRS){
        maxClearTimer = 1;
        game->maxClearDelay = 1;
    }else{
        maxClearTimer = 20;
        game->maxClearDelay = 20;
    }

    enableBlend((0b101010 << 8) + (1 << 6) + (0b1010));

    setPalette();

    showZoneMeter();
}

void GameScene::update(){

    if(refreshSkin){
        refreshSkin = false;
        setSkin();
        drawFrameBackgrounds();
    }

    if(!counted){
        countdown();
    }

    diagnose();

    if(exitGame){
        changeScene(new newTitleScene(),Transitions::FADE);
        exitGame = false;
        enableBot = false;
        return;
    }

    if (!game->lost && !paused && !game->eventLock) {
        // profile_start();
        game->update();
        // log(std::to_string(profile_stop()));
    }
    handleMultiplayer(0);

    progressBar(0);

    if(game->gameMode == BATTLE && !multiplayer){

        std::string test;
        if(!botGame->clearLock){
            testBot->run();
            botGame->update();
        }
        handleBotGame();
    }else if(enableBot){
        testBot->run();
    }

    if(creditRefresh && !game->clearLock)
        refreshCredits();

    gameSeconds = game->timer;

    if (game->clearLock && !(game->eventLock && game->gameMode != MASTER)) {
        clearTimer++;

        if(clearTimer >= 100){
            game->removeClearLock();
            // shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
            #ifdef GBA
            shake = (shakeMax * (savefile->settings.shakeAmount) / 4);
            #endif

            #ifdef PC
            shakeVelocity = int2fx(((shakeMax * (savefile->settings.shakeAmount) / 4))) * 2;
            #endif
            rumbleTimer = rumbleMax * 2 * savefile->settings.rumble;
            clearTimer = 0;
            updateText();
        }
    }

    if(game->eventLock){
        if(eventPauseTimer == 0){
            if(game->gameMode == MASTER){
                showBackground(0);
                eventPauseTimer = eventPauseTimerMax;
                setupCredits();
                flashTimer = flashTimerMax;
                gradient(false);
            // } else if(game->gameMode == MARATHON){
            //     flashTimer = flashTimerMax;
            //     eventPauseTimer = flashTimerMax + 2;
            }
        }

        if(eventPauseTimer){
            eventPauseTimer--;

            if(eventPauseTimer == 0)
                game->removeEventLock();
        }
    }

    latestDrop = game->getDrop();

    if (latestDrop.on){
        addGlow(latestDrop);
        addPlaceEffect(latestDrop);
    }

    canDraw = true;

    if (clearTimer >= maxClearTimer || maxClearTimer <= 0) {
        game->removeClearLock();

        #ifdef GBA
        shake = (shakeMax * (savefile->settings.shakeAmount) / 4);
        #endif

        #ifdef PC
        shakeVelocity = int2fx(((shakeMax * (savefile->settings.shakeAmount) / 4)) / 2);
        #endif

        // shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
        rumbleTimer = rumbleMax * 2 * savefile->settings.rumble;
        clearTimer = 0;
        updateText();
    }

    //TODO:
    #ifdef GBA
    rumble_update();
    if(rumbleTimer > 0){
        rumbleTimer--;

        rumble_set_state(rumble_start);
    }else{
        rumble_set_state(rumble_hard_stop);
    }
    #endif

    if ((game->won || game->lost) && !(flashTimer || eventPauseTimer)){
        paused = true;
        endScreen();
        paused = false;
        if(!(playAgain && multiplayer))
            return;
        else
            playAgain = false;
    }

    if (paused){
        if(pauseMenu()){
            paused = false;
            return;
        }
        liftKeys();
    }

    if (playAgain) {
        playAgain = false;
        paused = true;
        startGame();
        changeScene(new GameScene());
        return;
    }

    if(game->zoneTimer){
        if(gameSeconds % 4 == 0){
            frameSnow(0);
        }

        if(!flashTimer)
            rainbowPalette();
    }

    if(game->gameMode == MARATHON && game->subMode && gameSeconds % 2 == 0){
        if(rainbowIncreasing)
            rainbowTimer++;
        else
            rainbowTimer--;

        if(rainbowTimer >= (32 * 2) - 1 || rainbowTimer == 0)
            rainbowIncreasing = !rainbowIncreasing;

        showZoneMeter();
    }

    if(game->gameMode == MASTER && game->eventTimer){
        showCredits();
    }

    if(flashTimer)
        zoneFlash();

    sqran(qran() % frameCounter);

    if(++gridUpdateTimer >= gridUpdateTimerMax){
        gridUpdateTimer = 0;
        updateGrid();

        FIXED temp[22][12];

        memcpy32(temp, current, 22 * 12);
        memcpy32(current, previous, 22 * 12);
        memcpy32(previous, temp, 22 * 12);
    }

    if(dampTimer){
        dampTimer--;
        if(!dampTimer)
            damp = 2;
    }
}

void GameScene::deinit(){
    gradient(0);

    setPalette();
}
