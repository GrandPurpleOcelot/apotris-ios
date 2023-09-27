#include <string>

#include "def.h"
#include "platform.hpp"
#include "blockEngine.h"
#include "sprites.h"

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

#include "menu.h"
#include "scene.hpp"

#include "font1tiles_bin.h"
#include "font2tiles_bin.h"

using namespace BlockEngine;

const std::string save_type = "SRAM_Vnnn";
int getClassicPalette();
void setPawnPalette(int dest, int n, int blend);

OBJ_ATTR obj_buffer[128];

int frameCounter = 1;

int shake = 0;

int gameSeconds;

bool paused = false;

int marathonClearTimer = 20;

int initialLevel = 0;

bool canDraw = false;

int profileResults[10];

void addToResults(int input, int index);

Save* savefile = nullptr;

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

#ifdef GBA
u16 gradientTable[192 + 1];
#endif

#ifndef GBA
u16 gradientTable[SCREEN_HEIGHT + 1];
#endif

bool customSongs = false;
int customSongCount = 0;

const std::list<std::string> controlOptions = {
    "Move Left",
    "Move Right",
    "Rotate Left",
    "Rotate Right",
    "Rotate 180",
    "Soft Drop",
    "Hard Drop",
    "Hold",
    "Activate Zone",
};

const std::list<std::string> menuControlOptions = {
    "Up",
    "Down",
    "Left",
    "Right",
    "Confirm",
    "Cancel",
    "Pause",
    "Reset",
    "Special 1",
    "Special 2",
    "Special 3",
};


std::map<int,std::string> keyMap = {
    {KEY_LEFT,"Left"},
    {KEY_RIGHT,"Right"},
    {KEY_UP,"Up"},
    {KEY_DOWN,"Down"},
    {KEY_A,"A"},
    {KEY_B,"B"},
    {KEY_L,"L"},
    {KEY_R,"R"},
    {KEY_SELECT,"Select"},
};

void initialize(){
    initRumble();

    toggleSprites(true);

    buildBG(0, 0, 25, 0, 2, true);
    buildBG(1, 0, 26, 0, 3, true);
    buildBG(2, 2, 29, 0, 0, true);
    buildBG(3, 0, 27, 0, 1, true);

    for(int i = 0; i < 4; i++){
        toggleBG(i,true);
        setLayerScroll(i, 0, 0);
    }

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
    loadTiles(0,34,sprite45tiles_bin,sprite45tiles_bin_size/32);
    loadTiles(0,35,sprite46tiles_bin,sprite46tiles_bin_size/32);
    loadTiles(0,36,sprite47tiles_bin,sprite47tiles_bin_size/32);

    for(int i = 0; i < 8; i++)
        loadSpriteTiles(512 + 128 + i, moveSpriteTiles[i],1,1);

    clearTiles(2, 0, 1);
    loadTiles(2,1,font1tiles_bin,font1tiles_bin_size/32);

    setSkin();
    setPalette();
    setClearEffect();

    initFallingBlocks();

    setGradient(GRADIENT_COLOR);
}

void genWidths(){
    const int len = 96;

    int output[len][2];

    output[0][0] = 0;
    output[0][1] = 0;

    for(int i = 0; i < len; i++){
        TILE * t = (TILE *) &font1tiles_bin[i * 32];

        int found = 0;
        for(int x = 7; x >= 0; x--){
            for(int y = 0; y < 8; y++){
                if((t->data[y] >> (4 * x)) & 0xf){
                    output[i][1] = x;
                    found = 1;
                }

                if(found)
                    break;
            }
            if(found)
                break;
        }
    }

    for(int i = 0; i < len; i++){
        TILE * t = (TILE *) &font1tiles_bin[i * 32];

        int found = 0;
        for(int x = 0; x < 8; x++){
            for(int y = 0; y < 8; y++){
                if((t->data[y] >> (4 * x)) & 0xf){
                    output[i][0] = x;
                    found = 1;
                }

                if(found)
                    break;
            }
            if(found)
                break;
        }
    }

    std::string str;
    for(int i = 0; i < len; i++){
        output[i][1] -= output[i][0] - 1;

        str += "{" + std::to_string(output[i][0]) + ", " + std::to_string(output[i][1]) + "}, ";

        if(((i+1) % 8) == 0){
            log(str);
            str = "";
        }
    }

}

int main(void) {
    platformInit();

    loadSave();

    initialize();

    changeScene(new newTitleScene());

    //start screen animation
    while(closed()){
        vsync();
        scene->update();
    }

    deinitialize();
}

#ifdef PC
int WinMain(void){
    main();
    return 0;
}
#endif

std::string timeToString(int frames, bool small) {
    int t = (int)frames * 0.0167f;
    int millis = (int)(frames * 1.67f) % 100;
    int seconds = t % 60;
    int minutes = t / 60;

    char res[30];

    if(!small)
        posprintf(res,"%02d:%02d.%02d",minutes,seconds,millis);
    else
        posprintf(res,"%02d:%02d",minutes,seconds);

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

    shake = 0;
    shakeBuff = 0;
    shakeVelocity = 0;
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

    if(saveExists){
        delete quickSave;
        saveExists = false;
    }

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

    memset32(current, 0, 22 * 12);
    memset32(previous, 0, 22 * 12);
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

    MenuKeys k = savefile->settings.menuKeys;

    while (closed()) {
        vsync();

        if (place == 0 && game->gameMode != BATTLE && (game->won || game->gameMode == MARATHON || game->gameMode >= BLITZ))
            aprint("New Record", 10, 5);

        aprint("Name: ", 11, nameHeight - 2);

        aprint("DONE", 14, 16);

        key_poll();

        aprintClearArea(13, nameHeight, 10, 1);
        aprint(result, 13, nameHeight);

        if (!onDone) {
            #if defined(GBA) || defined(SWITCH)
            if (key_hit(k.confirm) || key_hit(k.right)) {
                cursor++;
                if (cursor > 7) {
                    onDone = true;
                    cursor = 7;
                    sfx(SFX_MENUCONFIRM);
                } else {
                    sfx(SFX_MENUMOVE);
                }
            }

            if (key_hit(k.cancel)) {
                result[cursor] = ' ';
                if (cursor > 0)
                    cursor--;
                sfx(SFX_MENUCANCEL);
            }

            if (key_hit(k.left)) {
                if (cursor > 0)
                    cursor--;
                sfx(SFX_MENUMOVE);
            }

            char curr = result.at(cursor);
            if (key_hit(k.up)) {
                if (curr == 'A')
                    result[cursor] = ' ';
                else if (curr == ' ')
                    result[cursor] = 'Z';
                else if (curr > 'A')
                    result[cursor] = curr - 1;

                sfx(SFX_MENUMOVE);
            } else if (key_hit(k.down)) {
                if (curr == 'Z')
                    result[cursor] = ' ';
                else if (curr == ' ')
                    result[cursor] = 'A';
                else if (curr < 'Z')
                    result[cursor] = curr + 1;
                sfx(SFX_MENUMOVE);
            } else if (key_is_down(k.up) || key_is_down(k.down)) {
                if (das < maxDas)
                    das++;
                else {
                    if (arr++ > maxArr) {
                        arr = 0;
                        if (key_is_down(k.up)) {
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

            #elif defined(PC)
            for(int i = 0; i < 26; i++){
                if(key_hit(i)){
                    result[cursor] = keyToString[i][0];
                    if(cursor < 7)
                        cursor++;
                    sfx(SFX_MENUMOVE);
                    break;
                }
            }

            if(key_hit(sf::Keyboard::Backspace) || (das == maxDas && arr == 0)){
                result[cursor] = ' ';
                if(cursor > 0){
                    cursor--;

                    sfx(SFX_MENUMOVE);
                }
            }

            if(key_is_down(sf::Keyboard::Backspace)){
                if(das < maxDas)
                    das++;

                if(++arr >= maxArr){
                    arr = 0;
                }
            }else{
                das = 0;
            }

            if(key_hit(sf::Keyboard::Space) && result.size() < 9){
                result[cursor] = ' ';
                if(cursor < 7)
                    cursor++;
                sfx(SFX_MENUMOVE);
            }

            if (timer++ > 19)
                timer = 0;

            if (timer < 10)
                aprint("_", 13 + cursor, nameHeight);

            if (key_hit(sf::Keyboard::Enter) || key_hit(sf::Keyboard::Escape)) {
                onDone = true;
                sfx(SFX_MENUCONFIRM);
            }

            #endif

            if (key_hit(k.pause)) {
                onDone = true;
                sfx(SFX_MENUCONFIRM);
            }

            aprint(" ", 12, 16);
        } else {
            aprint(">", 12, 16);

            if (key_hit(k.confirm) || key_hit(k.pause)) {
                sfx(SFX_MENUCONFIRM);
                break;
            }

            if (key_hit(k.cancel)) {
                onDone = false;
                sfx(SFX_MENUCANCEL);
            }
        }
        updateFluid();
        drawGrid();
    }

    clearText();

    if (result.size() >= 8)
        result = result.substr(0, 8);

    for (int i = 0; i < 8; i++)
        savefile->latestName[i] = result.at(i);

    return result;
}


void setSkin() {

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

        blockSprite = (u8*)classicTiles[0][0];
        break;
    case 8:
        for(int i = 0; i < 8; i++)
            loadTiles(0,48+i,classicTiles[1][i],classic1tiles_bin_size/32);

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
        break;
    case 12:
        blockSprite = (u8*)&sprite39tiles_bin[12*32];

        loadTiles(0,128,sprite39tiles_bin,sprite39tiles_bin_size/32);
        break;
    case 13:
        blockSprite = (u8*)&sprite40tiles_bin[12*32];

        loadTiles(0,128,sprite40tiles_bin,sprite40tiles_bin_size/32);
        break;
    case 14:
        blockSprite = (u8*)&sprite53tiles_bin[12*32];

        loadTiles(0,128,sprite53tiles_bin,sprite53tiles_bin_size/32);
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

    if(savefile->settings.skin < 7 || savefile->settings.skin == 9 || savefile->settings.skin == 10){
        //TODO:
        //vsync might not be necessary here
        vsync();
        int n = savefile->settings.skin;
        if(savefile->settings.skin < 0){
            n *= -1;
            n--;
            buildMini(&savefile->customSkins[n].smallBoard);
        }else{
            if (n >= 9)
                n-=2;
            buildMini((TILE*)mini[n]);
        }
    }

    loadTiles(0,1,blockSprite,1);
    loadTiles(2,97,blockSprite,1);

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
                if (board[ix]) {
                    if(savefile->settings.skin == 11)
                        loadSpriteTilesPartial(iy, &sprite38tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1,4);
                    else if(savefile->settings.skin == 12)
                        loadSpriteTilesPartial(iy, &sprite39tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1,4);
                    else if(savefile->settings.skin == 13)
                        loadSpriteTilesPartial(iy, &sprite40tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1,4);
                    else if(savefile->settings.skin == 14)
                        loadSpriteTilesPartial(iy, &sprite53tiles_bin[GameInfo::connectedConversion[(board[ix])>>4]*32],k,j,1,1,4);
                    else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                        loadSpriteTilesPartial(iy, blockSprite,k,j,1,1,4);
                    else
                        loadSpriteTilesPartial(iy, classicTiles[savefile->settings.skin-7][i],k,j,1,1,4);
                }else{
                    clearSpriteTile(iy, k, j, 4);
                }
            }
        }

        delete[] board;
    }
}

void setLightMode() {
    if (savefile->settings.lightMode) {
        setPaletteColor(0,0,0x5ad6,1);//background gray
        setPaletteColor(8,4,0x0421,1);//progressbar
        setPaletteColor(0,8,0x6739,1);

        //activated font
        // setPaletteColor(15,2,0x0000,1);//font main
        // setPaletteColor(15,3,0x5ad6,1);//font shaddow
        // setPaletteColor(31,2,0x0000,1);//obj font main
        // setPaletteColor(31,3,0x5ad6,1);//obj font shaddow

        //unactivated font
        // setPaletteColor(14,2,0x7fff,1);//font main
        // setPaletteColor(14,3,0x5ad6,1);//font shadow
        // setPaletteColor(30,2,0x7fff,1);//obj font main
        // setPaletteColor(30,3,0x5ad6,1);//obj font shadow

        setPaletteColor(16 + 8,6,0x6739,1);//frame background
    } else {
        setPaletteColor(0,0,0x0000,1);//background gray
        setPaletteColor(8,4,0x7fff,1);//progressbar
        setPaletteColor(0,8,0x0c63,1);

        //activated font
        // setPaletteColor(15,2,0x7fff,1);//font main
        // setPaletteColor(15,3,0x294a,1);//font shaddow
        // setPaletteColor(31,2,0x7fff,1);//obj font main
        // setPaletteColor(31,3,0x294a,1);//obj font shaddow

        //unactivated font
        // setPaletteColor(14,2,0x318c,1);//font main
        // setPaletteColor(14,3,0x0421,1);//font shadow
        // setPaletteColor(30,2,0x318c,1);//obj font main
        // setPaletteColor(30,3,0x0421,1);//obj font shadow

        setPaletteColor(16 + 8,6,0x0c63,1);//frame background
    }

    setGradient(savefile->settings.backgroundGradient);
}

void diagnose() {
    if (!DIAGNOSE)
        return;

}

void setPalette(){
    int n = (savefile->settings.colors < 2)?savefile->settings.colors:0;

    for(int i = 0; i < 16; i++){

        if (i < 8 && savefile->settings.colors < 2){
            loadPalette(i, 0, &palette[n][i*16], 8);
            loadPalette(i+16, 0, &palette[n][i*16], 8);
        }else if (i < 8){
            loadPalette(i, 4, &palette[n][i*16+4], 4);
            loadPalette(i+16, 4, &palette[n][i*16+4], 4);
        }else if (i > 9){
            loadPalette(i, 0, &palette[n][i*16], 16);
            loadPalette(i+16, 0, &palette[n][i*16], 16);
        }
    }

    int color = savefile->settings.palette + 2 * (savefile->settings.palette > 6);

    if(savefile->settings.colors == 2){
        for(int i = 0; i < 9; i++){
            loadPalette(i, 0, classic_pal_bin, 4);
            loadPalette(i+16, 0, classic_pal_bin, 4);
        }
    }else if(savefile->settings.colors == 3){
        //set frame color
        loadPalette(8, 0, &palette[n][color*16], 16);
        loadPalette(8+16, 0, &palette[n][color*16], 16);

        int n = getClassicPalette();

        for(int i = 0; i < 8; i++){
            loadPalette(i,1, &nesPalette[n][0],4);
            loadPalette(i+16,1, &nesPalette[n][0],4);
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
        }

        // if(!savefile->settings.lightMode){
        //     setPaletteColor(7, 0, 0x7fff, 2);
        //     setPaletteColor(7+16, 0, 0x7fff, 2);
        // }else{
        //     setPaletteColor(7, 0, 0x0421, 2);
        //     setPaletteColor(7+16, 0, 0x0421, 2);
        // }

    }else if(savefile->settings.colors == 5){
        for(int i = 0; i < 8; i++){
            loadPalette(i, 1, &arsPalette[0][i], 4);
            loadPalette(i+16, 1, &arsPalette[0][i], 4);
        }

        //set frame color
        loadPalette(8, 0, &palette[0][color*16], 16);
        loadPalette(8+16, 0, &palette[0][color*16], 16);
    }else if(savefile->settings.colors == 6){
        for(int i = 0; i < 8; i++){
            loadPalette(i, 1, &arsPalette[1][i], 4);
            loadPalette(i+16, 1, &arsPalette[1][i], 4);
        }
        //set frame color
        loadPalette(8, 0, &palette[1][color*16], 16);
        loadPalette(8+16, 0, &palette[1][color*16], 16);
    }else{
        //set frame color
        loadPalette(8, 0, &palette[n][color*16], 16);
        loadPalette(8+16, 0, &palette[n][color*16], 16);
    }

    setLightMode();

    //faded Text Palette
    color_fade_palette(13 + 16, 0, &palette[1][14*16], 0, 8, 15);
    color_fade_palette(13, 0, &palette[1][14*16], 0, 8, 15);

    if(savefile->settings.effects == 2)
        for(int i = 0; i < 16; i++)
            setPaletteColor(i, 5, RGB15(i, i, i), 1);

    setPaletteColor(15, 5, RGB15(25, 5, 5), 1);
}

int getClassicPalette(){
    int n = 0;

    if(game == nullptr){
        n = abs(savefile->seed) % 10;
        return n;
    }

    int mode = game->gameMode;
    if((mode == MARATHON || mode == BLITZ || mode == CLASSIC ) && game->level >= 0 && game->level < 255)
        n = (game->level-(game->gameMode != CLASSIC)) % 10;
    else
        n = abs(game->initSeed) % 10;

    return n;
}

void buildMini(TILE * customSkin){
    for(int i = 0; i < 7; i++)
        clearSpriteTiles(9*16 + 8 * i, 4, 2);

    int add = 0;
    int rs = SRS;
    if(game != nullptr){
        add = (game->rotationSystem == NRS || game->rotationSystem == ARS);
        rs = game->rotationSystem;
    }

    for (int i = 0; i < 7; i++){
        int* p = getShape(i, 0, rs);
        int tileStart = 9 * 16 + i * 8;

        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 4; x++){
                if(!p[(y+add)*4+x])
                    continue;

                for(int ii = 0; ii < 6; ii++){
                    for(int jj = 0; jj < 6; jj++){
                        setSpritePixel(tileStart, (x * 6 + jj)/8, (y*6+ii)/8, 4, ((x*6+jj)%8), (y*6+ii)%8, (customSkin->data[ii] >> (4*jj)) & 0xf);
                    }
                }
            }
        }

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
        t.initialType = savefile->settings.initialType,
    };

    return t;
}

void setGradient(int color){
    stopDMA();

    int div = 20;

    int start = 0;
    int end = 160;

    COLOR src = 0;

    #ifdef TE

    start = rowStart;
    end = rowEnd;

    #endif

    if(savefile->settings.backgroundType == 0)
        div = (end-start)/8;
    else if(savefile->settings.backgroundType == 1)
        div = (end-start)/16;

    COLOR color1 = color & 0xffff;
    COLOR color2 = (color >> 16) & 0xffff;

    if(!(savefile->settings.lightMode && color == 0)){
        for(int i = 0; i < end-start; i++){
            if(savefile->settings.backgroundType == 0)
                color_fade((COLOR *) &gradientTable[i+start], &src, color, 1, (((end-start)-1-i) / div) * 2 + 16);
            else if(savefile->settings.backgroundType == 1)
                color_fade((COLOR *) &gradientTable[i+start], &color2, color1, 1, (((end-start)-1-i) / div) * 2);
        }

        #ifdef GBA
        memset16(&gradientTable[SCREEN_HEIGHT-1], gradientTable[0], 192 - SCREEN_HEIGHT);
        // gradientTable[SCREEN_HEIGHT-1] = gradientTable[0];
        // gradientTable[192-1] = gradientTable[0];
        #endif
    }else{
        for(int i = 0; i < end-start; i++)
            gradientTable[i + start] = 0x5ad6;
    }
}

void setDefaultGradient(){
    #ifdef GBA
    memcpy16(gradientTable,defaultGradient_bin,defaultGradient_bin_size/2);
    #endif

    #ifdef PC

    int div = SCREEN_HEIGHT/8;
    int n = (savefile->settings.colors < 2)?savefile->settings.colors:0;

    COLOR * src = (COLOR *) &palette[n];

    for(int i = 0; i < SCREEN_HEIGHT; i++){
        color_fade((COLOR *) &gradientTable[i],src,GRADIENT_COLOR,1,((SCREEN_HEIGHT-1-i) / div) * 2 + 16);
    }

    #endif
}

void gradient(int state){
    if(state == 1){
        gradientEnabled = true;
        bool light = savefile->settings.lightMode;
        if((!light && gradientTable[0] == 0) || (light && gradientTable[0] == 0x5ad6)){
            setPaletteColor(0, 0, gradientTable[0], 1);
            return;
        }
        toggleHBlank(false);
    }else if(state == 2){
        stopDMA();
        gradientEnabled = false;
        toggleHBlank(true);
    }else{
        stopDMA();
        gradientEnabled = false;
        toggleHBlank(false);
    }
}

void startGame(BlockEngine::Options options,int seed){
    if(game != nullptr)
        delete game;
    game = new Game(options.mode,seed,options.bigMode);
    game->setOptions(options);

    savefile->stats.gamesStarted++;

    if(previousGameOptions != nullptr)
        delete previousGameOptions;
    previousGameOptions = new Options();

    *previousGameOptions = options;

    if (!replaying && enableBot) {
        log("starting bot!");

        delete testBot;
        testBot = new Bot(game);
    }else if(options.mode == BATTLE && !multiplayer){
        startBotGame(seed);
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
    dest += 16;

    if (!savefile->settings.lightMode){
        if(savefile->settings.colors == 2)
            color_fade_palette(dest,0,(COLOR*)classic_pal_bin,0x0000,5,blend);
        else if(savefile->settings.colors == 3){
            int n = getClassicPalette();
            color_fade_palette(dest,1, &nesPalette[n][0],0x0000,4,blend);
        }else if(savefile->settings.colors == 4){
            COLOR c;
            if(savefile->settings.palette < 7){
                int *clr = (int *) GameInfo::colors[savefile->settings.palette];
                c = RGB15_SAFE(clr[0] >> 3, clr[1] >> 3, clr[2] >> 3);
            } else{
                c = monoPalette[savefile->settings.lightMode][0];
            }
            COLOR mono[4];
            memset16(mono,c,4);

            color_fade_palette(dest,1,(COLOR*)&mono[0],0x0000,4,blend);
        }else if(savefile->settings.colors == 5){
            color_fade_palette(dest,1,(COLOR*)&arsPalette[0][n],0x0000,4,blend);
        }else if(savefile->settings.colors == 6){
            color_fade_palette(dest,1,(COLOR*)&arsPalette[1][n],0x0000,4,blend);
        } else
            color_fade_palette(dest,0,(COLOR*)&palette[savefile->settings.colors][n*16],0x0000,5,blend);
    }else{
        if(dest == 11+16)
            blend = int2fx(blend) >> 5;
        else if(dest == 10+16)
            blend = float2fx(0.25);

        if(savefile->settings.colors == 2)
            color_adj_brightness(dest, 0, (COLOR*)classic_pal_bin, 5, blend);
        else if(savefile->settings.colors == 3){
            color_adj_brightness(dest, 1, (COLOR*)&nesPalette[getClassicPalette()][0], 4, blend);
        }else if(savefile->settings.colors == 4){
            COLOR c;
            if(savefile->settings.palette < 7){
                int *clr = (int *) GameInfo::colors[savefile->settings.palette];
                c = RGB15_SAFE(clr[0] >> 3, clr[1] >> 3, clr[2] >> 3);
            } else{
                c = monoPalette[savefile->settings.lightMode][0];
            }
            COLOR mono[4];
            memset16(mono,c,4);

            color_adj_brightness(dest, 1, (COLOR*)&mono[0], 4, blend);
        }else if(savefile->settings.colors == 5){
            color_adj_brightness(dest, 1, (COLOR*)&arsPalette[0][n], 4, blend);
        }else if(savefile->settings.colors == 6){
            color_adj_brightness(dest, 1, (COLOR*)&arsPalette[1][n], 4, blend);
        }else
            color_adj_brightness(dest, 0, (COLOR*)&palette[savefile->settings.colors][n * 16], 5, blend);
    }
}

std::string getStringFromKey(int key){
    #if defined(GBA)

    std::string result;

    if((key & (key - 1)) == 0){
        result = keyMap[key];
    }else{

        int k = key;

        int counter = 0;
        do{
            if(k & (1 << counter)){
                result+= keyMap[1<<counter] + " ";
                k-= 1 << counter;
            }

            counter++;
        }while(k != 0);

        //remove trailing space
        result.pop_back();
    }

    if(result != ""){
        return result;
    }else{
        return "?";
    }

    #elif defined(PC)

    u8 k = key & 0xff;

    if(k >= 0 && k < sf::Keyboard::KeyCount)
        return keyToString[k];

    #elif defined(SWITCH) || defined(MM)

    return keyToString[key];

    #endif

    return "?";

}
