#include <string>

#include "def.h"
#include "sprite37tiles_bin.h"
#include "sprite5tiles_bin.h"
#include "tetrisEngine.h"
#include "sprites.h"

// #include "soundbank.h"
// #include "soundbank_bin.h"

#include "LinkConnection.h"

#include "logging.h"

#include "text.h"

#include "flashSaves.h"

#include "posprintf.h"
#include "rumble.h"

#include "classic1tiles_bin.h"
#include "classic_pal_bin.h"

#include "tetromino.hpp"

#include "defaultGradient_bin.h"

#include <bit>

using namespace Tetris;

int getClassicPalette();
void setPawnPalette(int dest, int n, int blend);

OBJ_ATTR obj_buffer[128];

int frameCounter = 1;

int shake = 0;

int gameSeconds;

bool pause = false;

int marathonClearTimer = 20;

int initialLevel = 0;

bool canDraw = false;

int profileResults[10];

void addToResults(int input, int index);

Save* savefile;

u8* blockSprite;

bool restart = false;

bool playAgain = false;
int nextSeed = 0;

bool multiplayer = false;
bool playingBotGame = false;

bool rumble_enabled = false;

bool rumbleInitialized = false;

Scene * scene = nullptr;

bool gradientEnabled = false;

Options *previousGameOptions = nullptr;

// bool resumeJourney = false;
// bool journeyLevelUp = false;
// bool journeySaveExists = false;
// Tetris::Game* journeySave;

u16 gradientTable[SCREEN_HEIGHT + 1];

// void onHBlank() {
//     if(REG_VCOUNT < 160)
//         pal_bg_mem[0] = gradientTable[REG_VCOUNT];
// }

void initialize(){
    toggleSprites(true);

    buildBG(0, 0, 25, 0, 2, true);
    buildBG(1, 0, 26, 0, 3, true);
    buildBG(2, 2, 29, 0, 0, true);
    buildBG(3, 0, 27, 0, 1, true);


    for(int i = 0; i < 4; i++){
        toggleBG(i,true);
        setLayerScroll(i, 0, 0);
    }

    // REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2) | BG_MOSAIC;
    // REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3) | BG_MOSAIC;
    // REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0) | BG_MOSAIC;
    // REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
    // REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering

    //Load bg tiles

    loadTiles(0,2,sprite3tiles_bin,sprite3tiles_bin_size/32);
    loadTiles(0,4,sprite5tiles_bin,sprite5tiles_bin_size/32);
    loadTiles(0,6,sprite8tiles_bin,sprite8tiles_bin_size/32);
    loadTiles(0,12,sprite10tiles_bin,sprite10tiles_bin_size/32);
    loadTiles(0,13,sprite11tiles_bin,sprite11tiles_bin_size/32);
    loadTiles(0,14,sprite12tiles_bin,sprite12tiles_bin_size/32);
    loadTiles(0,15,sprite13tiles_bin,sprite13tiles_bin_size/32);
    loadTiles(0,26,sprite17tiles_bin,sprite17tiles_bin_size/32);
    loadTiles(0,27,sprite20tiles_bin,sprite20tiles_bin_size/32);
    loadTiles(0,30,sprite22tiles_bin,sprite22tiles_bin_size/32);
    loadTiles(0,31,sprite23tiles_bin,sprite23tiles_bin_size/32);
    loadTiles(0,32,sprite24tiles_bin,sprite24tiles_bin_size/32);
    loadTiles(0,33,sprite42tiles_bin,sprite42tiles_bin_size/32);

    // memcpy16(&tile_mem[0][2], sprite3tiles_bin, sprite3tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][4], sprite5tiles_bin, sprite3tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][6], sprite8tiles_bin, sprite8tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][12], sprite10tiles_bin, sprite10tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][13], sprite11tiles_bin, sprite11tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][14], sprite12tiles_bin, sprite12tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][15], sprite13tiles_bin, sprite13tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][26], sprite17tiles_bin, sprite17tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][27], sprite20tiles_bin, sprite20tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][30], sprite22tiles_bin, sprite22tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][31], sprite23tiles_bin, sprite23tiles_bin_size / 2);
    // memcpy16(&tile_mem[0][32], sprite24tiles_bin, sprite24tiles_bin_size / 2);

    loadSpriteTiles(512, sprite6tiles_bin,4,4);

    loadSpriteTiles(512 + 16, sprite6tiles_bin,4,3);
    loadSpriteTilesPartial(512 + 16, &sprite6tiles_bin[128], 0, 3,4,1);
    loadSpriteTiles(512 + 32, &sprite6tiles_bin[128],4,2);
    loadSpriteTilesPartial(512 + 32, &sprite6tiles_bin[128],0,2,4,2);
    loadSpriteTiles(512 + 48, &sprite6tiles_bin[128],4,1);
    loadSpriteTilesPartial(512 + 48, &sprite6tiles_bin[128],0,1,4,3);

    loadSpriteTiles(512 + 64, title1tiles_bin,8,8);
    loadSpriteTiles(512 + 96, title2tiles_bin,8,8);

    for(int i = 0; i < 8; i++)
        loadSpriteTiles(512 + 128 + i, moveSpriteTiles[i],1,1);

    loadTiles(2,0,fontTiles,fontTilesLen/32);

    //hold tiles
    // memcpy16(&tile_mem[5][0], sprite6tiles_bin, sprite6tiles_bin_size / 2);

    //queue frame Tiles

    // memcpy16(&tile_mem[5][16], sprite6tiles_bin, sprite6tiles_bin_size / 2 * 3 / 4);
    // memcpy16(&tile_mem[5][16 + 12], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 1 / 4);
    // memcpy16(&tile_mem[5][32], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 2 / 4);
    // memcpy16(&tile_mem[5][32 + 8], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 2 / 4);
    // memcpy16(&tile_mem[5][48], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 1 / 4);
    // memcpy16(&tile_mem[5][48 + 4], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 3 / 4);

    // memcpy16(&tile_mem[5][64], title1tiles_bin, title1tiles_bin_size / 2);
    // memcpy16(&tile_mem[5][96], title2tiles_bin, title2tiles_bin_size / 2);

    // for(int i = 0; i < 8; i++)
    //     memcpy16(&tile_mem[5][128+i], moveSpriteTiles[i], 16);

    // memcpy16(&tile_mem[2][0], fontTiles, fontTilesLen / 2);

    //load tetriminoes into tile memory for menu screen animation

    setSkin();
    setClearEffect();


    // REG_BLDCNT = BLD_BUILD(BLD_BG1,BLD_BACKDROP,BLD_STD);

    // enableLayerBlend(1);

    // REG_BLDCNT = (1 << 6) + (1 << 13) + (1 << 1);
    // REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(2);

    setGradient(GRADIENT_COLOR);
}

int main(void) {

    loadSave();

    platformInit();

    // TLN_SetBGColor(50, 50, 50);
    // while(closed()){
    //     vsync();


    //     key_poll();

    //     u16 key = key_hit(KEY_FULL);

    //     if(key)
    //         log(std::to_string(key));
    // }

    initialize();



    //start screen animation
    while(closed){
        reset();

        if(!playAgain){
            startScreen();
		}else{
            playAgain = false;

            if(game->gameMode != BATTLE){
                startGame();
            }else{
                int seed = qran();

                startGame(seed);
                startBotGame(seed);
            }
            // int goal = game->goal;
            // int training = game->trainingMode;
            // int rs = game->rotationSystem;

            // delete game;
            // game = new Game(game->gameMode,bigMode);
            // game->setGoal(goal);
            // game->setLevel(initialLevel);
            // game->setTuning(getTuning());
            // game->setTrainingMode(training);
            // game->pawn.big = bigMode;
            // game->bTypeHeight = goalSelection;
            // game->setSubMode(subMode);
            // game->setRotationSystem(rs);
        }

        gameLoop();
    }
}

std::string timeToString(int frames) {
    int t = (int)frames * 0.0167f;
    int millis = (int)(frames * 1.67f) % 100;
    int seconds = t % 60;
    int minutes = t / 60;

	char res[30];

	posprintf(res,"%02d:%02d:%02d",minutes,seconds,millis);

    std::string result = "";

    result = res;

    return result;
}


void addToResults(int input, int index) {
    profileResults[index] = input;
}

void reset() {
    clearSprites(128);
    clearText();
    setLayerScroll(0, 0, 0);
    setLayerScroll(1, 0, 0);
    // REG_BG0HOFS = 0;
    // REG_BG0VOFS = 0;
    // REG_BG1HOFS = 0;
    // REG_BG1VOFS = 0;
    shake = 0;
    push = 0;

    floatingList.clear();
    placeEffectList.clear();
    rumbleTimer = 0;
    //TODO:
    #ifdef GBA
    rumble_set_state(RumbleState(rumble_stop));
    #endif

    resetZonePalette();

    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);

    // memset32(&se_mem[25], 0x0000, 32 * 20);
    // memset32(&se_mem[26], 0x0000, 32 * 20);
    // memset32(&se_mem[27], 0x0000, 32 * 20);

    setMosaic(0, 0);

    //reset glow
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            glow[i][j] = 0;

    int g = savefile->settings.backgroundGradient;

    if(g == 0)
        memcpy16(gradientTable,defaultGradient_bin,defaultGradient_bin_size/2);
    else
        setGradient(g);
}

std::string nameInput(int place) {

    std::string result = savefile->latestName;
    int cursor = 0;

    for (int i = 0; i < (int)result.size(); i++) {
        if (result[i] == ' ') {
            cursor = i;
            break;
        }
    }

    const static int nameHeight = 14;

    int timer = 0;

    int das = 0;
    int maxDas = 12;

    int arr = 0;
    int maxArr = 5;

    bool onDone = false;

    showSprites(128);

    while (closed()) {
        vsync();
        // VBlankIntrWait();

        if (place == 0 && game->gameMode != BATTLE && (game->won || game->gameMode == MARATHON || game->gameMode >= BLITZ))
            aprint("New Record", 10, 5);

        aprint("Name: ", 11, nameHeight - 2);

        aprint("DONE", 14, 16);

        key_poll();

        u16 key = key_hit(KEY_FULL);

        aprint(result, 13, nameHeight);

        if (!onDone) {
            if (key == KEY_A || key == KEY_RIGHT) {
                cursor++;
                if (cursor > 7) {
                    onDone = true;
                    cursor = 7;
                    sfx(SFX_MENUCONFIRM);
                } else {
                    sfx(SFX_MENUMOVE);
                }
            }

            if (key == KEY_B) {
                result[cursor] = ' ';
                if (cursor > 0)
                    cursor--;
                sfx(SFX_MENUCANCEL);
            }

            if (key == KEY_LEFT) {
                if (cursor > 0)
                    cursor--;
                sfx(SFX_MENUMOVE);
            }

            if (key == KEY_START) {
                onDone = true;
                sfx(SFX_MENUCONFIRM);
            }

            char curr = result.at(cursor);
            if (key == KEY_UP) {
                if (curr == 'A')
                    result[cursor] = ' ';
                else if (curr == ' ')
                    result[cursor] = 'Z';
                else if (curr > 'A')
                    result[cursor] = curr - 1;

                sfx(SFX_MENUMOVE);
            } else if (key == KEY_DOWN) {
                if (curr == 'Z')
                    result[cursor] = ' ';
                else if (curr == ' ')
                    result[cursor] = 'A';
                else if (curr < 'Z')
                    result[cursor] = curr + 1;
                sfx(SFX_MENUMOVE);
            } else if (key_is_down(KEY_UP) || key_is_down(KEY_DOWN)) {
                if (das < maxDas)
                    das++;
                else {
                    if (arr++ > maxArr) {
                        arr = 0;
                        if (key_is_down(KEY_UP)) {
                            if (curr == 'A')
                                result[cursor] = ' ';
                            else if (curr == ' ')
                                result[cursor] = 'Z';
                            else if (curr > 'A')
                                result[cursor] = curr - 1;
                        } else {
                            if (curr == 'Z')
                                result[cursor] = ' ';
                            else if (curr == ' ')
                                result[cursor] = 'A';
                            else if (curr < 'Z')
                                result[cursor] = curr + 1;
                        }
                        sfx(SFX_MENUMOVE);
                    }
                }
            } else {
                das = 0;
                if (timer++ > 19)
                    timer = 0;

                if (timer < 10)
                    aprint("_", 13 + cursor, nameHeight);
            }

            aprint(" ", 12, 16);
        } else {
            aprint(">", 12, 16);

            if (key == KEY_A || key == KEY_START) {
                sfx(SFX_MENUCONFIRM);
                break;
            }

            if (key == KEY_B) {
                onDone = false;
                sfx(SFX_MENUCANCEL);
            }
        }
    }

    clearText();

    if (result.size() >= 8)
        result = result.substr(0, 8);

    for (int i = 0; i < 8; i++)
        savefile->latestName[i] = result.at(i);

    return result;
}


void setSkin() {
    setPalette();

    switch (savefile->settings.skin) {
    case 0:
        blockSprite = (u8*)sprite1tiles_bin;
        break;
    case 1:
        blockSprite = (u8*)sprite7tiles_bin;
        break;
    case 2:
        blockSprite = (u8*)sprite9tiles_bin;
        break;
    case 3:
        blockSprite = (u8*)sprite14tiles_bin;
        break;
    case 4:
        blockSprite = (u8*)sprite18tiles_bin;
        break;
    case 5:
        blockSprite = (u8*)sprite19tiles_bin;
        break;
    case 6:
        blockSprite = (u8*)sprite21tiles_bin;
        break;
    case 7:

        for(int i = 0; i < 8; i++)
            loadTiles(0,48+i,classicTiles[0][i],classic1tiles_bin_size/32);
        //     memcpy16(&tile_mem[0][48+i],classicTiles[0][i],classic1tiles_bin_size/2);

        blockSprite = (u8*)classicTiles[0][0];
        break;
    case 8:
        for(int i = 0; i < 8; i++)
            loadTiles(0,48+i,classicTiles[1][i],classic1tiles_bin_size/32);
            // memcpy16(&tile_mem[0][48+i],classicTiles[1][i],classic1tiles_bin_size/2);

        blockSprite = (u8*)classicTiles[1][0];
        break;
    case 9:
        blockSprite = (u8*)sprite27tiles_bin;
        break;
    case 10:
        blockSprite = (u8*)sprite28tiles_bin;
        break;
    case 11:
        blockSprite = (u8*)&sprite38tiles_bin[12*32];

        loadTiles(0,128,sprite38tiles_bin,sprite38tiles_bin_size/32);
        // memcpy32(&tile_mem[0][128],sprite38tiles_bin,sprite38tiles_bin_size/4);
        break;
    case 12:
        blockSprite = (u8*)&sprite39tiles_bin[12*32];

        loadTiles(0,128,sprite39tiles_bin,sprite39tiles_bin_size/32);
        // memcpy32(&tile_mem[0][128],sprite39tiles_bin,sprite39tiles_bin_size/4);
        break;
    case 13:
        blockSprite = (u8*)&sprite40tiles_bin[12*32];

        loadTiles(0,128,sprite40tiles_bin,sprite40tiles_bin_size/32);
        // memcpy32(&tile_mem[0][128],sprite40tiles_bin,sprite40tiles_bin_size/4);
        break;
    default:
        if(savefile->settings.skin < 0){
            int n = savefile->settings.skin;
            n *= -1;
            n--;

            blockSprite = (u8*)&savefile->customSkins[n].board;
        }
        break;
    }


    if(savefile->settings.skin < 7){
        //TODO:
        //vsync might not be necessary here
        vsync();
        // VBlankIntrWait();
        if(savefile->settings.skin < 0){
            int n = savefile->settings.skin;
            n *= -1;
            n--;
            buildMini(&savefile->customSkins[n].smallBoard);
        }else{
            buildMini((TILE*)mini[savefile->settings.skin]);
        }
    }


    loadTiles(0,1,blockSprite,1);
    loadTiles(2,97,blockSprite,1);

    // memcpy16(&tile_mem[0][1], blockSprite, sprite1tiles_bin_size / 2);
    // memcpy16(&tile_mem[2][97], blockSprite, sprite1tiles_bin_size / 2);

    int* board;
    for (int i = 0; i < 7; i++) {
        int n;
        if(game == nullptr)
            n = SRS;
        else
            n = game->rotationSystem;

        board = getShape(i, 0, n);

        int iy = 16 * i;
        for (int j = 0; j < 4; j++) {

            for (int k = 0; k < 4; k++) {
                int ix = j * 4 + k;
                // int index = iy + ix;
                if (board[ix]) {
                    if(savefile->settings.skin == 11)
                        loadSpriteTilesPartial(iy, &sprite38tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1);
                    else if(savefile->settings.skin == 12)
                        loadSpriteTilesPartial(iy, &sprite39tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1);
                    else if(savefile->settings.skin == 13)
                        loadSpriteTilesPartial(iy, &sprite40tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1);
                    else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                        loadSpriteTilesPartial(iy, blockSprite,k,j,1,1);
                    else
                        loadSpriteTilesPartial(iy, blockSprite,k,j,1,1);
                        // memcpy16(&tile_mem[4][index], classicTiles[savefile->settings.skin-7][i], sprite1tiles_bin_size / 2);
                    // if(savefile->settings.skin == 11)
                    //     memcpy16(&tile_mem[4][index], &sprite38tiles_bin[GameInfo::connectedConversion[(board[ix])>>4] * 32], sprite1tiles_bin_size / 2);
                    // else if(savefile->settings.skin == 12)
                    //     memcpy16(&tile_mem[4][index], &sprite39tiles_bin[GameInfo::connectedConversion[(board[ix])>>4] * 32], sprite1tiles_bin_size / 2);
                    // else if(savefile->settings.skin == 13)
                    //     memcpy16(&tile_mem[4][index], &sprite40tiles_bin[GameInfo::connectedConversion[(board[ix])>>4] * 32], sprite1tiles_bin_size / 2);
                    // else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                    //     memcpy16(&tile_mem[4][index], blockSprite, sprite1tiles_bin_size / 2);
                    // else
                    //     memcpy16(&tile_mem[4][index], classicTiles[savefile->settings.skin-7][i], sprite1tiles_bin_size / 2);
                }else{
                    clearSpriteTile(iy, k, j, 4);
                    // clearSpriteTiles(0,1,1);
                    // memcpy16(&tile_mem[4][index], &tile_mem[4][0], sprite1tiles_bin_size / 2);
                }
            }
        }

        // for (int i = 0; i < 4; i++)
        //     delete[] board[i];
        delete[] board;
    }
}

void setLightMode() {

    if (savefile->settings.lightMode) {
        setPaletteColor(0,0,0x5ad6,1);//background gray
        setPaletteColor(8,4,0x0421,1);//progressbar

        //activated font
        setPaletteColor(15,2,0x0000,1);//font main
        setPaletteColor(15,3,0x5ad6,1);//font shaddow
        setPaletteColor(31,2,0x0000,1);//obj font main
        setPaletteColor(31,3,0x5ad6,1);//obj font shaddow

        //unactivated font
        setPaletteColor(14,2,0x7fff,1);//font main
        setPaletteColor(14,3,0x5ad6,1);//font shadow
        setPaletteColor(30,2,0x7fff,1);//obj font main
        setPaletteColor(30,3,0x5ad6,1);//obj font shadow

        // memset16(pal_bg_mem, 0x5ad6, 1);//background gray
        // memset16(&pal_bg_mem[8 * 16 + 4], 0x0421, 1);//progressbar

        // //activated font
        // memset16(&pal_bg_mem[15 * 16 + 2], 0x0000, 1);//font main
        // memset16(&pal_bg_mem[15 * 16 + 3], 0x5ad6, 1);//font shadow
        // memset16(&pal_obj_mem[15 * 16 + 2], 0x0000, 1);//obj font main
        // memset16(&pal_obj_mem[15 * 16 + 3], 0x5ad6, 1);//boj font shadow

        // //unactivated font
        // memset16(&pal_bg_mem[14 * 16 + 2], 0x7fff, 1);//font main
        // memset16(&pal_bg_mem[14 * 16 + 3], 0x5ad6, 1);//font shadow
        // memset16(&pal_obj_mem[14 * 16 + 2], 0x7fff, 1);//obj font main
        // memset16(&pal_obj_mem[14 * 16 + 3], 0x5ad6, 1);//boj font shadow
    } else {
        setPaletteColor(0,0,0x0000,1);//background gray
        setPaletteColor(8,4,0x7fff,1);//progressbar

        //activated font
        setPaletteColor(15,2,0x7fff,1);//font main
        setPaletteColor(15,3,0x294a,1);//font shaddow
        setPaletteColor(31,2,0x7fff,1);//obj font main
        setPaletteColor(31,3,0x294a,1);//obj font shaddow

        //unactivated font
        setPaletteColor(14,2,0x318c,1);//font main
        setPaletteColor(14,3,0x0421,1);//font shadow
        setPaletteColor(30,2,0x318c,1);//obj font main
        setPaletteColor(30,3,0x0421,1);//obj font shadow

        setPaletteColor(16 + 8,6,0x0c63,1);//frame background

        // memset16(pal_bg_mem, 0x0000, 1);
        // memset16(&pal_bg_mem[8 * 16 + 4], 0x7fff, 1);

        // //activated font
        // memset16(&pal_bg_mem[15 * 16 + 2], 0x7fff, 1);
        // memset16(&pal_bg_mem[15 * 16 + 3], 0x294a, 1);
        // memset16(&pal_obj_mem[15 * 16 + 2], 0x7fff, 1);
        // memset16(&pal_obj_mem[15 * 16 + 3], 0x294a, 1);

        // //unactivated font
        // memset16(&pal_bg_mem[14 * 16 + 2], 0x318c, 1);//font main
        // memset16(&pal_bg_mem[14 * 16 + 3], 0x0421, 1);//font shadow
        // memset16(&pal_obj_mem[14 * 16 + 2], 0x318c, 1);//obj font main
        // memset16(&pal_obj_mem[14 * 16 + 3], 0x0421, 1);//boj font shadow
    }

    setGradient(savefile->settings.backgroundGradient);
}

void diagnose() {
    if (!DIAGNOSE)
        return;

    // std::string str = "";

    // for(int i = 0; i < 10; i++){
    //     str += std::to_string(game->columnHeights[i]) + " ";
    // }

    // log(str);

    // str+= "Garbage Height: " + std::to_string(game->garbageHeight);

    // str += std::to_string(game->previousBest.size()) + " " + std::to_string(game->moveHistory.size());

    // str += std::to_string(profileResults[0]);

}

void setPalette(){
    int n = (savefile->settings.colors < 2)?savefile->settings.colors:0;

    for(int i = 0; i < 16; i++){

        if (i < 8 && savefile->settings.colors < 2){
            loadPalette(i, 0, &palette[n][i*16], 8);
            loadPalette(i+16, 0, &palette[n][i*16], 8);
            // memcpy16(&pal_bg_mem[i*16], &palette[n][i*16], 8);
            // memcpy16(&pal_obj_mem[i*16], &palette[n][i*16], 8);
        }else if (i < 8){
            loadPalette(i, 4, &palette[n][i*16], 8);
            loadPalette(i+16, 4, &palette[n][i*16], 8);
            // memcpy16(&pal_bg_mem[i*16+4], &palette[n][i*16+4], 4);
            // memcpy16(&pal_obj_mem[i*16+4], &palette[n][i*16+4], 4);
        }else if (i > 9){
            loadPalette(i, 0, &palette[n][i*16], 16);
            loadPalette(i+16, 0, &palette[n][i*16], 16);
            // memcpy16(&pal_bg_mem[i*16], &palette[n][i*16], 16);
            // memcpy16(&pal_obj_mem[i*16], &palette[n][i*16], 16);
        }
    }

    int color = savefile->settings.palette + 2 * (savefile->settings.palette > 6);

    if(savefile->settings.colors == 2){
        for(int i = 0; i < 9; i++){
            loadPalette(i, 0, classic_pal_bin, 4);
            loadPalette(i+16, 0, classic_pal_bin, 4);
            // memcpy16(&pal_bg_mem[i*16], classic_pal_bin,4);
            // memcpy16(&pal_obj_mem[i*16], classic_pal_bin,4);
        }
    }else if(savefile->settings.colors == 3){
        //set frame color
        loadPalette(8, 0, &palette[n][color*16], 16);
        loadPalette(8+16, 0, &palette[n][color*16], 16);
        // memcpy16(&pal_obj_mem[8 * 16], &palette[n][color * 16], 16);
        // memcpy16(&pal_bg_mem[8 * 16], &palette[n][color * 16], 16);

        int n = getClassicPalette();
        for(int i = 0; i < 8; i++){
            loadPalette(i,1, &nesPalette[n][0],4);
            loadPalette(i+16,1, &nesPalette[n][0],4);
            // memcpy16(&pal_bg_mem[i*16+1], &nesPalette[n][0],4);
            // memcpy16(&pal_obj_mem[i*16+1], &nesPalette[n][0],4);
        }

    }else if(savefile->settings.colors == 4){
        COLOR c;
        if(savefile->settings.palette < 7){
            int *clr = (int *) GameInfo::colors[savefile->settings.palette];
            c = RGB15_SAFE(clr[0] >> 3, clr[1] >> 3, clr[2] >> 3);
        } else
            c = monoPalette[savefile->settings.lightMode][0];

        for(int i = 0; i < 9; i++){
            setPaletteColor(i, 1, c, 4);
            setPaletteColor(i+16, 1, c, 4);
            // memset16(&pal_bg_mem[i*16+1], c,4);
            // memset16(&pal_obj_mem[i*16+1], c,4);
        }

        if(!savefile->settings.lightMode){
            setPaletteColor(7, 0, 0x7fff, 2);
            setPaletteColor(7+16, 0, 0x7fff, 2);
            // memset16(&pal_bg_mem[7], 0x7fff,2);
            // memset16(&pal_obj_mem[7], 0x7fff,2);
        }else{
            setPaletteColor(7, 0, 0x0421, 2);
            setPaletteColor(7+16, 0, 0x0421, 2);
            // memset16(&pal_bg_mem[7], 0x0421,2);
            // memset16(&pal_obj_mem[7], 0x0421,2);
        }

    }else if(savefile->settings.colors == 5){
        for(int i = 0; i < 8; i++){
            loadPalette(i, 1, &arsPalette[0][i], 4);
            loadPalette(i+16, 1, &arsPalette[0][i], 4);
            // memcpy16(&pal_bg_mem[i*16+1], &arsPalette[0][i],4);
            // memcpy16(&pal_obj_mem[i*16+1], &arsPalette[0][i],4);
        }

        //set frame color
        loadPalette(8, 0, &palette[0][color*16], 16);
        loadPalette(8+16, 0, &palette[0][color*16], 16);
        // memcpy16(&pal_obj_mem[8 * 16], &palette[0][color * 16], 16);
        // memcpy16(&pal_bg_mem[8 * 16], &palette[0][color * 16], 16);
    }else if(savefile->settings.colors == 6){
        for(int i = 0; i < 8; i++){
            loadPalette(i, 1, &arsPalette[1][i], 4);
            loadPalette(i+16, 1, &arsPalette[1][i], 4);
            // memcpy16(&pal_bg_mem[i*16+1], &arsPalette[1][i],4);
            // memcpy16(&pal_obj_mem[i*16+1], &arsPalette[1][i],4);
        }
        //set frame color
        loadPalette(8, 0, &palette[1][color*16], 16);
        loadPalette(8+16, 0, &palette[1][color*16], 16);
        // memcpy16(&pal_obj_mem[8 * 16], &palette[1][color * 16], 16);
        // memcpy16(&pal_bg_mem[8 * 16], &palette[1][color * 16], 16);
    }else{
        //set frame color
        loadPalette(8, 0, &palette[n][color*16], 16);
        loadPalette(8+16, 0, &palette[n][color*16], 16);
        // memcpy16(&pal_obj_mem[8 * 16], &palette[n][color * 16], 16);
        // memcpy16(&pal_bg_mem[8 * 16], &palette[n][color * 16], 16);
    }

    loadPalette(13+16, 0, title_pal_bin, title_pal_bin_size/2);
    // memcpy16(&pal_obj_mem[13 * 16], title_pal_bin, title_pal_bin_size / 2);
    setLightMode();
}

int getClassicPalette(){
    int n = 0;
    int mode = game->gameMode;
    if((mode == MARATHON || mode == BLITZ || mode == CLASSIC ) && game->level >= 0 && game->level < 255)
        n = (game->level-(game->gameMode != CLASSIC)) % 10;
    else
        n = abs(game->initSeed) % 10;

    return n;
}

void buildMini(TILE * customSkin){
    //TODO:
    //lengthX and Y might be wrong on pc
    clearSpriteTiles(9*16, 8, 7);

    // memset32(&tile_mem[4][9*16],0,8*8*7);

    int add = 0;
    int rs = SRS;
    if(game != nullptr){
        add = (game->rotationSystem == NRS || game->rotationSystem == ARS);
        rs = game->rotationSystem;
    }

    for (int i = 0; i < 7; i++){

        // TILE * t;
        int* p = getShape(i, 0, rs);
        int tileStart = 9 * 16 + i * 8;

        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 4; x++){
                if(!p[(y+add)*4+x])
                    continue;

                for(int ii = 0; ii < 6; ii++){
                    for(int jj = 0; jj < 6; jj++){
                        setSpritePixel(tileStart, (x * 6 + jj)/8, (y*6+ii)/8, 4, ((x*6+jj)%8), (y*6+ii)%8, (customSkin->data[ii] >> (4*jj)) & 0xf);
                        // t = &tile_mem[4][tileStart + (x*6+jj)/8 + ((y*6+ii)/8)*4];

                        // t->data[(y*6+ii)%8] |= ((customSkin->data[ii] >> (4*jj)) & 0xf) << (((x*6+jj)%8)*4);
                    }
                }
            }
        }

        // for(int i = 0; i < 4; i++)
        //     delete p[i];
        delete p;
    }
}

Tuning getTuning(){
    Tuning t = {
        t.das = savefile->settings.das,
        t.arr = savefile->settings.arr,
        t.sfr = savefile->settings.sfr,
        t.dropProtection = savefile->settings.dropProtectionFrames,
        t.directionalDas = savefile->settings.directionalDas,
        t.delaySoftDrop = savefile->settings.delaySoftDrop,
        t.ihs = savefile->settings.ihs,
        t.irs = savefile->settings.irs,
    };

    return t;
}

void changeScene(Scene *newScene){
    if(scene != nullptr){
        delete scene;
    }

    scene = newScene;
}

void setGradient(int color){
    stopDMA();

    int n = (savefile->settings.colors < 2)?savefile->settings.colors:0;

    COLOR * src = (COLOR *) &palette[n];

    int div = 20;

    #ifdef PC

    div = SCREEN_HEIGHT/8;

    #endif

    if(!(savefile->settings.lightMode && color == 0)){
        for(int i = 0; i < SCREEN_HEIGHT; i++){
            color_fade((COLOR *) &gradientTable[i],src,color,1,((SCREEN_HEIGHT-1-i) / div) * 2 + 16);
            // color_fade(src, color, (COLOR *) &gradientTable[i], 1, ((SCREEN_HEIGHT-1-i) / 20) * 2 + 16);
        }

        #ifdef GBA
        gradientTable[SCREEN_HEIGHT-1] = gradientTable[0];
        #endif
    }else{
        for(int i = 0; i < SCREEN_HEIGHT; i++)
            gradientTable[i] = 0x5ad6;
    }

}

void setDefaultGradient(){
    memcpy16(gradientTable,defaultGradient_bin,defaultGradient_bin_size/2);
}

void gradient(int state){
    stopDMA();
    // if(!state){
    //     REG_DMA0CNT &= ~(1 << 0x1f);
    // }
    if(state == 1){
        gradientEnabled = true;
        toggleHBlank(false);
    }else if(state == 2){
        gradientEnabled = false;
        toggleHBlank(true);
    }else{
        gradientEnabled = false;
    }
}

void startGame(Tetris::Options options,int seed){
    if(game != nullptr)
        delete game;
    game = new Game(options.mode,seed,options.bigMode);

    game->setOptions(options);

    if(previousGameOptions != nullptr)
        delete previousGameOptions;
    previousGameOptions = new Options();

    *previousGameOptions = options;

    if (ENABLE_BOT) {
        delete testBot;
        testBot = new Bot(game);
    }
}

void startGame(int seed){
    if(previousGameOptions == nullptr)
        previousGameOptions = new Options();

    startGame(*previousGameOptions,seed);
}

void startGame(){
    if(previousGameOptions == nullptr)
        previousGameOptions = new Options();

    startGame(*previousGameOptions,qran());
}

void startBotGame(int seed){
    startMultiplayerGame(seed);
    multiplayer = false;

    delete botGame;
    botGame = new Game(BATTLE,seed & 0x1fff,false);
    botGame->setGoal(100);
    game->setTuning(getTuning());
    botGame->setLevel(1);
    botGame->maxClearDelay = 20;

    delete testBot;
    testBot = new Bot(botGame);

    clearText();
}

void setPawnPalette(int dest, int n, int blend){
    // COLOR c;
    // if(savefile->settings.palette < 7){
    //     int *clr = (int *) GameInfo::colors[savefile->settings.palette];
    //     c = RGB15_SAFE(clr[0] >> 3, clr[1] >> 3, clr[2] >> 3);
    // } else
    //     c = monoPalette[savefile->settings.lightMode][0];
    dest += 16;

    if (!savefile->settings.lightMode){
        if(savefile->settings.colors == 2)
            color_fade_palette(dest,0,(COLOR*)classic_pal_bin,0x0000,8,blend);
            // clr_fade((COLOR*)classic_pal_bin, 0x0000, &pal_obj_mem[dest * 16], 8, blend);
        else if(savefile->settings.colors == 3){
            color_fade_palette(dest,1,(COLOR*)classic_pal_bin,0x0000,4,blend);
            // clr_fade((COLOR*)&nesPalette[getClassicPalette()][0], 0x0000, &pal_obj_mem[dest * 16+1], 4, blend);
        }else if(savefile->settings.colors == 4){
            color_fade_palette(dest,1,(COLOR*)&monoPalette[0][0],0x0000,4,blend);
            // clr_fade((COLOR*)&monoPalette[0][0], 0x0000, &pal_obj_mem[dest * 16+1], 4, blend);
        }else if(savefile->settings.colors == 5){
            color_fade_palette(dest,1,(COLOR*)&arsPalette[0][n],0x0000,4,blend);
            // clr_fade((COLOR*)&arsPalette[0][n], 0x0000, &pal_obj_mem[dest * 16+1], 4, blend);
        }else if(savefile->settings.colors == 6){
            color_fade_palette(dest,1,(COLOR*)&arsPalette[1][n],0x0000,4,blend);
            // clr_fade((COLOR*)&arsPalette[1][n], 0x0000, &pal_obj_mem[dest * 16+1], 4, blend);
        } else
            color_fade_palette(dest,0,(COLOR*)&palette[savefile->settings.colors][n*16],0x0000,8,blend);
            // clr_fade_fast((COLOR*)&palette[savefile->settings.colors][n * 16], 0x0000, &pal_obj_mem[dest * 16], 8, blend);
    }else{
        if(dest == 11+16)
            blend = int2fx(blend) >> 5;
        else if(dest == 10+16)
            blend = float2fx(0.25);

        if(savefile->settings.colors == 2)
            color_adj_brightness(dest, 0, (COLOR*)classic_pal_bin, 8, blend);
            // clr_adj_brightness(&pal_obj_mem[dest * 16], (COLOR*)classic_pal_bin, 8, blend);
        else if(savefile->settings.colors == 3){
            color_adj_brightness(dest, 1, (COLOR*)&nesPalette[getClassicPalette()][0], 8, blend);
            // clr_adj_brightness(&pal_obj_mem[dest * 16+1], (COLOR*)&nesPalette[getClassicPalette()][0], 8, blend);
        }else if(savefile->settings.colors == 4){
            color_adj_brightness(dest, 1, (COLOR*)&monoPalette[1][0], 4, blend);
            // clr_adj_brightness(&pal_obj_mem[dest * 16+1], (COLOR*)&monoPalette[1][0], 4, blend);
        }else if(savefile->settings.colors == 5){
            color_adj_brightness(dest, 1, (COLOR*)&arsPalette[0][n], 4, blend);
            // clr_adj_brightness(&pal_obj_mem[dest * 16+1], (COLOR*)&arsPalette[0][n], 4, blend);
        }else if(savefile->settings.colors == 6){
            color_adj_brightness(dest, 1, (COLOR*)&arsPalette[1][n], 4, blend);
            // clr_adj_brightness(&pal_obj_mem[dest * 16+1], (COLOR*)&arsPalette[1][n], 4, blend);
        }else
            color_adj_brightness(dest, 0, (COLOR*)&palette[savefile->settings.colors][n * 16], 8, blend);
            // clr_adj_brightness(&pal_obj_mem[dest * 16], (COLOR*)&palette[savefile->settings.colors][n * 16], 8, blend);
    }
}
