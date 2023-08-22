#include "def.h"
#include "menu.h"
#include "sprite41tiles_bin.h"
#include "blockEngine.h"
#include "tetromino.hpp"
#include "text.h"
#include <string>
#include <list>
#include "LinkConnection.h"
#include "flashSaves.h"

#include "posprintf.h"
#include "logging.h"
#include "sprites.h"
#include "classic_pal_bin.h"
#include "scene.hpp"

#include "sceneGraphics.hpp"
#include "sceneAudio.hpp"
#include "sceneControls.hpp"
#include "sceneHandling.hpp"
#include "sceneModes.hpp"
#include "sceneStats.hpp"

std::string timeToStringHours(int frames);
void initFallingBlocks();

WordSprite* wordSprites[MAX_WORD_SPRITES];

int titleFloat = 0;
OBJ_ATTR* titleSprites[2];

#ifdef GBA
const int bgGridHeight = 30;
#endif
#ifdef PC
const int bgGridHeight = 42;
#endif
u16 backgroundArray[bgGridHeight][30];

int bgSpawnBlock = 0;
int bgSpawnBlockMax = 30;
int gravity = 0;
int gravityMax = 10;
int previousOptionScreen = -1;
bool goToOptions = false;
int previousSelection = 0;
int previousOptionMax = 0;

// std::list<int> keyHistory;

Settings previousSettings;
Skin previousSkins[MAX_CUSTOM_SKINS];

bool bigMode = false;

const std::list<std::string> menuOptions = {"Play","Settings","Stats","Links","Skin Editor","Credits"};
const std::list<std::string> gameOptions = {"Marathon","Sprint","Dig","Ultra","Blitz","Combo","Survival","Classic","Master","Zen","2P Battle","CPU Battle","Training"};
const std::list<std::string> settingsOptions = {"Graphics","Audio","Controls","Handling","Gameplay"};

// const int secretCombo[11] = {KEY_UP,KEY_UP,KEY_DOWN,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_LEFT,KEY_RIGHT,KEY_B,KEY_A,KEY_START};

bool proMode = false;

bool demo = false;

std::string previousElement;

std::list<std::string> path;

void showTitleSprites() {
    for (int i = 0; i < 2; i++)
        titleSprites[i] = &obj_buffer[14 + i];
    for (int i = 0; i < 2; i++) {
        sprite_unhide(titleSprites[i], 0);
        sprite_set_attr(titleSprites[i], ShapeWide, 3, 512 + 64 + i * 32, 12, 0);
        sprite_enable_affine(titleSprites[i], i, true);
        sprite_set_size(titleSprites[i], float2fx(0.5), i);

        int offset = (sinLut(titleFloat) * 3) >> 12;

        if(offset == 3)
            offset = 2;

        sprite_set_pos(titleSprites[i], 120 - 64 + 128 * i - 64, 24 + offset);
    }

    titleFloat+=3;
    if(titleFloat >= 512)
        titleFloat = 0;
}

void fallingBlocks() {
    gravity++;
    bgSpawnBlock++;

    int i, j;
    if (gravity > gravityMax) {
        gravity = 0;

        for (i = bgGridHeight - 1; i >= 0; i--) {
            for (j = 0; j < 30; j++) {
                if (i == 0)
                    backgroundArray[i][j] = 0;
                else
                    backgroundArray[i][j] = backgroundArray[i - 1][j];
            }
        }
    }

    bool bigMode = savefile->settings.big;

    int offset = 4 + 6 * bigMode;

    for (i = 0; i < (bgGridHeight - 10); i++) {
        for (j = 0; j < 30; j++) {
            if (backgroundArray[i+offset][j]){
                int n = (backgroundArray[i+offset][j] - 1) & 0xf;
                int r = backgroundArray[i+offset][j] >> 4;

                if(savefile->settings.skin >= 11)
                    setTile(25,j,i,tileBuild(128 + GameInfo::connectedConversion[r], false, false, n));
                    // *dest++ = 128 + GameInfo::connectedConversion[r] + ((n) << 12);
                else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                    setTile(25,j,i,tileBuild(1, false, false, n));
                    // *dest++ = (1 + ((n) << 12));
                else{
                    setTile(25,j,i,tileBuild(48, false, false, n));
                    // *dest++ = (48 + n + ((n) << 12));
                }
            }else{
                setTile(25,j,i,0);
                // *dest++ = 0;
            }
        }
        // dest += 2;
    }

    if (bgSpawnBlock > bgSpawnBlockMax) {

        int n = qran() % 7;
        int* p = BlockEngine::getShape(n, qran() % 4, BlockEngine::SRS);

        bool found = false;

        if(!bigMode){
            int x = qran() % 27;
            // int x = qran() % 7;
            // x += 20 * (qran() % 2);

            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    if (backgroundArray[i][j + x])
                        found = true;

            if(!found)
                for (i = 0; i < 4; i++)
                    for (j = 0; j < 4; j++)
                        if (p[i * 4 + j])
                            backgroundArray[i][j + x] = n + p[i * 4 + j];
        }else{
            int x = (qran() % 13) * 2;

            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++)
                    if (j + x * 2 >= 30 || backgroundArray[i][j + x*2])
                        found = true;

            if(!found){
                for (i = 0; i < 4; i++){
                    for (j = 0; j < 4; j++){
                        int xoffset = (j+x)*2;
                        int yoffset = i*2;

                        if (!p[i * 4 + j] || yoffset < 0 || yoffset > 23 || xoffset < 0 || xoffset > 29)
                            continue;

                        backgroundArray[yoffset][xoffset] = n + 1;
                        backgroundArray[yoffset][xoffset+1] = n + 1;
                        backgroundArray[yoffset+1][xoffset] = n + 1;
                        backgroundArray[yoffset+1][xoffset+1] = n + 1;
                    }
                }
            }
        }

        delete[] p;

        bgSpawnBlock = 0;
    }
}

std::string timeToStringHours(int frames) {
    int t = (int)frames * 0.0167f;
    int minutes = (t / 60)%60;
    int hours = t / 3600;

    char res[30];

    posprintf(res,"%02d:%02d",hours,minutes);

    std::string result = "";

    result = res;

    return result;
}

bool settingsChanged(){
    u8 * v1 = (u8 *) &savefile->settings;
    u8 * v2 = (u8 *) &previousSettings;

    for(int i = 0; i < (int) sizeof(Settings); i++){
        if(v1[i] != v2[i])
            return true;
    }

    return false;
}

void newTitleScene::init(){
    reset();
    toggleBG(3,false);

    loadSpriteTiles(512 + 64, title1tiles_bin,8,4);
    loadSpriteTiles(512 + 96, title2tiles_bin,8,4);

    loadPalette(12+16, 0, title_pal_bin, title_pal_bin_size/2);
    loadTiles(2,102,sprite37tiles_bin,sprite37tiles_bin_size/32);
    loadTiles(2,105,sprite41tiles_bin,sprite41tiles_bin_size/32);

    setSkin();
    setLightMode();

    clearSprites(128);

    playSongRandom(0);

    //backgroundGrid
    setTiles(26,0,32*32, tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));

    clearSpriteTiles(256, 12 * MAX_WORD_SPRITES, 1);
    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i].setup(i, 64 + i * 3, 256 + i * 12, false);

    wordSprites[0].setText("Press Any");
    wordSprites[1].setText("Button");

    versionText.setText("v4.0.0a");

    nameText.setText("akouzoukos");

    enableBlend((0b000000 << 8) + (1 << 6) + (1 << 3));
}

void newTitleScene::draw(){
    fallingBlocks();
    showTitleSprites();
    showSprites(128);

    const int y = 160 - (1) * 8;
    versionText.show(0, y, 15);
    nameText.show(240 - nameText.width, y, 15);

    if(flashTimer < flashMax/2){
        int w = wordSprites[0].width + wordSprites[1].width + 6;

        wordSprites[0].show((240 - w)/2,14*8,15);
        wordSprites[1].show((240 - w)/2 + wordSprites[0].width + 6,14*8,15);
    }else{
        wordSprites[0].hide();
        wordSprites[1].hide();
    }

    if(++flashTimer > flashMax-1){
        flashTimer = 0;
    }
}

bool newTitleScene::control(){
    if(key_hit(KEY_SELECT)){
        sfx(SFX_MENUCONFIRM);

        QuickStart q = savefile->quickStart;

        BlockEngine::Options newOptions;

        newOptions.mode = BlockEngine::Modes(q.mode);
        newOptions.goal = q.goal;
        newOptions.level = q.level;
        newOptions.tuning = getTuning();
        newOptions.bigMode = q.big;
        newOptions.subMode = q.submode;
        savefile->stats.gamesStarted++;

        if(newOptions.mode != BlockEngine::BATTLE){
            startGame(newOptions,qran());
        }else{
            int seed = qran();

            startGame(newOptions,seed);
            startBotGame(seed);
        }

        gameLoop();

        return true;
    }else if(key_hit(KEY_R)){
        loadReplay();
    }else if(key_hit(KEY_FULL)){
        sfx(SFX_MENUCONFIRM);
        changeScene(new MainMenuScene(),Transitions::FADE);
        return true;
    }

    return false;
}

void newTitleScene::update(){
    canDraw = 1;
    key_poll();

    if(control()){
        return;
    }

    demoTimer++;
    if(demoTimer >= demoMax){
        demoTimer = 0;
        enableBot = true;
        demo = true;

        botThinkingSpeed = 6;
        botSleepDuration = 5;
        botStepMax = 1;

        startGame(BlockEngine::Options(),qran());

        changeScene(new GameScene(), Transitions::FADE);
    }
}

void newTitleScene::deinit(){
    clearSprites(128);
    showSprites(128);

    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);
    clearSpriteTiles(2, 100, 1);
    clearSpriteTiles(256, 256, 1);

    clearText();
}

Scene* MainMenuScene::previousScene(){
    return new newTitleScene();
}

void ConfirmSaveScene::init(){
    clearSprites(128);
    showSprites(128);

    clearTilemap(29);
    naprint("Save changes?", 64 , 56);

    naprint("SAVE", 48, 104);
    naprint("DON'T SAVE", 120, 104);

    //backgroundGrid
    setTiles(26,0,32*32, tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));

    setTiles(27,0,32*32, tileBuild(34, false, false, 0));

    lengths[0] = getVariableWidth("SAVE");
    lengths[1] = getVariableWidth("DON'T SAVE");

    pos[0] = 48 + lengths[0] / 2;
    pos[1] = 120 + lengths[1] / 2;

    loadSpriteTiles(16*7, blockSprite, 1, 1);
    for(int i = 0; i < 2; i++){
        cursorSprites[i] = &obj_buffer[1 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_hide(cursorSprites[i]);
    }

    enableBlend((0b101011 << 8) + (1 << 6) + (1 << 3));
}

void ConfirmSaveScene::draw(){
    fallingBlocks();

    int offset = (sinLut(cursorFloat) * 2) >> 12;
    FIXED scale = float2fx((1.0 - ((float) 0.1 * offset)));

    const int y = 104;

    for(int i = 0; i < 2; i++){
        sprite_unhide(cursorSprites[i],0);
        sprite_enable_affine(cursorSprites[i], i, true);
        sprite_set_size(cursorSprites[i], scale , i);

        int x = pos[cancel] - ((lengths[cancel] + 8) / 2 + offset + 4) * ((i)?-1:1) - 8;

        sprite_set_pos(cursorSprites[i], x, y - 5);
    }

    showSprites(128);
}

void ConfirmSaveScene::update(){
    canDraw = 1;
    key_poll();

    cursorFloat += 6;
    if(cursorFloat >= 512)
        cursorFloat = 0;

    control();
}

bool ConfirmSaveScene::control(){
    if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
        cancel = !cancel;
        sfx(SFX_MENUMOVE);
    }

    if(key_hit(KEY_A)){
        sfx(SFX_MENUCONFIRM);

        if(cancel){
            savefile->settings = previousSettings;

            for (int i = 0; i < MAX_CUSTOM_SKINS; i++)
                savefile->customSkins[i] = previousSkins[i];

            setMusicVolume(512 * ((float)savefile->settings.volume / 10));
            setSkin();
            setLightMode();
            setGradient(savefile->settings.backgroundGradient);

        }else{
            saveSavefile();
        }

        changeScene(new MainMenuScene(),Transitions::FADE);
    }else if(key_hit(KEY_B)){
        sfx(SFX_MENUCANCEL);
        changeScene(new SettingsScene(),Transitions::FADE);
    }

    return false;
}

void MainMenuScene::init(){
    path.clear();
    SimpleListScene::init();
}

void initFallingBlocks(){
    memset16(backgroundArray, 0, 30 * (bgGridHeight));
}
