#include "def.h"
#include "platform.hpp"
#include "text.h"
// #include "soundbank.h"
#include "sprites.h"
#include <string>
#include "posprintf.h"
#include "logging.h"
#include "scene.hpp"
#include "sceneGraphics.hpp"

using namespace Tetris;

void showLabels();
void gradientEditor();

static const int startX = 5;
static const int endX = 24;

static const int startY = 2;
static const int options = 15;

void GraphicsScene::init(){
    gradient(2);

    setPalette();
    setGradient(savefile->settings.backgroundGradient);

    if (savefile->settings.lightMode)
        setPaletteColor(0, 0, 0x5ad6, 1);
        // memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        setPaletteColor(0, 0, 0x0000, 1);
        // memset16(pal_bg_mem, 0x0000, 1);

    clearTilemap(26);
    // memset16(&se_mem[26], 0x0000, 32 * 20);

    delete game;
    game = new Game(DIG,bigMode);
    game->setGoal(0);
    game->setLevel(1);
    game->rotationSystem = SRS;

    game->update();

    game->pawn.y++;

    toggleBG(0, true);
    toggleBG(1, true);
    toggleBG(2, true);
    toggleBG(3, true);

    enableBlend((1 << 6) + (0b111011 << 8) + (1 << 3));

    buildBG(3, 0, 27, 0, 1, 0);
    setTiles(27, 0, 32 * 32, tileBuild(12, false, false, 4 * savefile->settings.lightMode));
    showLabels();

    drawFrameBackgrounds();
}

bool GraphicsScene::control(){
    return false;
}

void GraphicsScene::update(){
    drawGrid();
    drawFrame(0);
    screenShake();
    showPlaceEffect();

    if(savefile->settings.floatText){
        showClearText();

        if(!floatingList.empty() && showOptions)
            showLabels();
    }

    key_poll();

    aprint("R: Toggle", 0, 18);
    aprint("Options", 3, 19);

    if (key_hit(KEY_START) || (key_hit(KEY_A) && selection != 0)) {
        sfx(SFX_MENUCONFIRM);
        if (selection != options - 1) {
            selection = options - 1;
        } else {
            return;
        }
        setSkin();
        // setLightMode();
    }

    if (key_hit(KEY_B)) {
        sfx(SFX_MENUCANCEL);
        if (selection == options - 1)
            selection = 0;
        else
            return;
    }

    if (key_hit(KEY_R)) {
        showOptions = !showOptions;

        if (!showOptions) {
            clearText();
            aprint("R: Toggle", 0, 18);
            aprint("Options", 3, 19);
            // enableBlend(blendInfo & ~ (1 << 0xc));
            enableBlend(prevBld);
        }else{
            showLabels();
            enableBlend((1 << 6) + (0b111011 << 8) + (1 << 3));
            // enableBlend(blendInfo | (1 << 0xc));
        }

        toggleBG(3, showOptions);
    }

    if (key_hit(KEY_UP)) {
        if (selection > 0)
            selection--;
        else
            selection = options - 1;
        sfx(SFX_MENUMOVE);
    }

    if (key_hit(KEY_DOWN)) {
        if (selection < options - 1)
            selection++;
        else
            selection = 0;
        sfx(SFX_MENUMOVE);
    }

    if(key_hit(KEY_SELECT)){
        setDefaultGraphics(savefile, 0);
        setSkin();
        setPalette();
        setTiles(27, 0, 32 * 32, tileBuild(12, false, false, 4 * savefile->settings.lightMode));
        // memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);
    }

    if (key_is_down(KEY_UP)) {
        if (dasVer < maxDas) {
            dasVer++;
        } else if(selection != 0){
            if (arr++ > maxArr) {
                arr = 0;
                selection--;
                sfx(SFX_MENUMOVE);
            }
        }
    } else if (key_is_down(KEY_DOWN)) {
        if (dasVer < maxDas) {
            dasVer++;
        } else if(selection != options-1){
            if (arr++ > maxArr) {
                arr = 0;
                selection++;
                sfx(SFX_MENUMOVE);
            }
        }
    } else {
        dasVer = 0;
    }

    if(selection == 2){
        if (key_is_down(KEY_LEFT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if(savefile->settings.skin+dasOffset > - MAX_CUSTOM_SKINS){
                if (arr++ > maxArr) {
                    arr = 0;
                    // savefile->settings.skin--;
                    dasOffset--;
                    sfx(SFX_MENUMOVE);
                }
            }
        } else if (key_is_down(KEY_RIGHT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if(savefile->settings.skin+dasOffset < MAX_SKINS-1){
                if (arr++ > maxArr) {
                    arr = 0;
                    // savefile->settings.skin++;
                    dasOffset++;
                    sfx(SFX_MENUMOVE);
                }
            }
        } else {
            if(dasHor == maxDas){
                savefile->settings.skin += dasOffset;
                dasOffset = 0;
                setSkin();
                showPawn();
                showShadow();
            }
            dasHor = 0;
        }
    }

    if(key_hit(KEY_A) && selection == 0){
        clearText();

        clearSprites(128);
        showSprites(128);
        // oam_init(obj_buffer, 128);
        // oam_copy(oam_mem, obj_buffer, 128);

        sfx(SFX_MENUCONFIRM);
        gradientEditor();
        if(showOptions){
            toggleBG(3, showOptions);
            showLabels();
        }

    }

    if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)) {
        switch (selection) {
        case 0:
            break;
        case 1:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.backgroundGrid > 0) {
                    savefile->settings.backgroundGrid--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1) {
                    savefile->settings.backgroundGrid++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }

            drawGrid();
            break;
        case 2:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.skin > -(MAX_CUSTOM_SKINS)) {
                    savefile->settings.skin--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.skin < MAX_SKINS - 1) {
                    savefile->settings.skin++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }

            setSkin();
            showPawn();
            showShadow();
            break;
        case 3:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.shadow > 0) {
                    savefile->settings.shadow--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.shadow < MAX_SHADOWS - 1) {
                    savefile->settings.shadow++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }
            break;
        case 4:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.palette > 0) {
                    savefile->settings.palette--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.palette < 7) {
                    savefile->settings.palette++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }
            setPalette();
            break;
        case 5:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.colors > 0) {
                    savefile->settings.colors--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.colors < MAX_COLORS-1) {
                    savefile->settings.colors++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }
            setPalette();
            break;
        case 6:
            savefile->settings.edges = !savefile->settings.edges;
            sfx(SFX_MENUMOVE);
            break;
        case 7:
            savefile->settings.lightMode = !savefile->settings.lightMode;
            setTiles(27, 0, 32 * 32, tileBuild(12, false, false, 4 * savefile->settings.lightMode));

            // memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);
            setSkin();
            // setLightMode();

            sfx(SFX_MENUMOVE);
            break;
        case 8:
            savefile->settings.effects = !savefile->settings.effects;
            if (savefile->settings.effects) {
                effectList.push_back(Effect(1, 4, 5));
            }
            sfx(SFX_MENUMOVE);
            break;
        case 9:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.clearEffect > 0) {
                    savefile->settings.clearEffect--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.clearEffect < MAX_CLEAR_EFFECTS - 1) {
                    savefile->settings.clearEffect++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }

            setClearEffect();
            break;
        case 10:
            #ifdef PC
            sfx(SFX_MENUCANCEL);
            break;
            #endif
            savefile->settings.placeEffect = !savefile->settings.placeEffect;
            if(savefile->settings.placeEffect && (int)placeEffectList.size() < 3){
                placeEffectList.push_back(PlaceEffect(game->pawn.x, game->pawn.y-20, 0, 0, game->pawn.current, 0, false));
            }

            sfx(SFX_MENUMOVE);
            break;
        case 11:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.shakeAmount > 0) {
                    savefile->settings.shakeAmount--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.shakeAmount < 4) {
                    savefile->settings.shake = true;
                    savefile->settings.shakeAmount++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }
            shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
            break;
        case 12:
            savefile->settings.floatText = !savefile->settings.floatText;

            if(savefile->settings.floatText){
                floatingList.push_back(FloatText("quad"));
            }else{
                for(auto const & floating : floatingList){
                    int height;
                    if (floating.timer < 2 * 100 / 3)
                        height = 5 * (float)floating.timer / ((float)2 * 100 / 3);
                    else
                        height = (30 * (float)(floating.timer) / 100) - 15;
                    aprintClearArea(9, 15-height, 12, 1);
                }

                if(showOptions)
                    showLabels();

                floatingList.clear();
            }
            sfx(SFX_MENUMOVE);
            break;
        case 13:
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.maxQueue > 1) {
                    savefile->settings.maxQueue--;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            } else {
                if (savefile->settings.maxQueue < 5) {
                    savefile->settings.maxQueue++;
                    sfx(SFX_MENUMOVE);
                } else {
                    sfx(SFX_MENUCANCEL);
                }
            }
            drawFrameBackgrounds();
            break;
        }
    }

    if (showOptions) {

        aprint(" DONE ", 12, 17);

        int counter = 0;

        for (int i = 0; i < options; i++)
            aprint("       ", endX - 2, startY + i);

        aprint("...", endX, startY + counter++);

        aprintf(savefile->settings.backgroundGrid + 1, endX, startY + counter++);

        if(savefile->settings.skin+dasOffset >= 0)
            aprintf(savefile->settings.skin + 1 + dasOffset, endX, startY + counter);
        else
            aprint("C" + std::to_string(-(savefile->settings.skin + dasOffset)), endX-1, startY + counter);

        counter++;

        aprintf(savefile->settings.shadow + 1, endX, startY + counter++);

        aprintf(savefile->settings.palette + 1, endX, startY + counter++);

        aprintf(savefile->settings.colors+1, endX, startY + counter++);

        if (savefile->settings.edges)
            aprint("ON", endX, startY + counter);
        else
            aprint("OFF", endX, startY + counter);

        counter++;

        if (savefile->settings.lightMode)
            aprint("ON", endX, startY + counter);
        else
            aprint("OFF", endX, startY + counter);

        counter++;

        if (savefile->settings.effects)
            aprint("ON", endX, startY + counter);
        else
            aprint("OFF", endX, startY + counter);

        counter++;

        aprintf(savefile->settings.clearEffect + 1, endX, startY + counter);

        counter++;

        #ifdef PC
        aprintColor("OFF", endX, startY + counter,1);

        #endif
        #ifdef GBA
        if (savefile->settings.placeEffect)
            aprint("ON", endX, startY + counter);
        else
            aprint("OFF", endX, startY + counter);
        #endif

        counter++;

        std::string shakeString = std::to_string(savefile->settings.shakeAmount * 25) + "%";
        aprint(shakeString, endX, startY + counter);

        counter++;

        if (savefile->settings.floatText)
            aprint("ON", endX, startY + counter);
        else
            aprint("OFF", endX, startY + counter);

        counter++;

        aprintf(savefile->settings.maxQueue, endX, startY + counter++);

        switch (selection) {
        case 0:
            aprint("[",endX-1,startY+selection);
            aprint("]",endX+3,startY+selection);
            break;
        case 1:
            if (savefile->settings.backgroundGrid > 0)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1)
                aprint(">", endX + 1, startY + selection);
            break;
        case 2:
            if (savefile->settings.skin+dasOffset > -(MAX_CUSTOM_SKINS))
                aprint("<", endX - 1 - (savefile->settings.skin+dasOffset < 0), startY + selection);
            if (savefile->settings.skin+dasOffset < MAX_SKINS - 1)
                aprint(">", endX + 1 + (savefile->settings.skin+dasOffset > 8), startY + selection);
            break;
        case 3:
            if (savefile->settings.shadow > 0)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.shadow < MAX_SHADOWS-1)
                aprint(">", endX + 1, startY + selection);
            break;
        case 4:
            if (savefile->settings.palette > 0)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.palette < 7)
                aprint(">", endX + 1, startY + selection);
            break;
        case 5:
            if (savefile->settings.colors > 1)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.colors < MAX_COLORS)
                aprint(">", endX + 1, startY + selection);
            break;
        case 6:
            aprint("[", endX - 1, startY + selection);
            aprint("]", endX + 2 + !(savefile->settings.edges), startY + selection);
            break;
        case 7:
            aprint("[", endX - 1, startY + selection);
            aprint("]", endX + 2 + !(savefile->settings.lightMode), startY + selection);
            break;
        case 8:
            aprint("[", endX - 1, startY + selection);
            aprint("]", endX + 2 + !(savefile->settings.effects), startY + selection);
            break;
        case 9:
            if (savefile->settings.clearEffect > 0)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.clearEffect < MAX_CLEAR_EFFECTS-1)
                aprint(">", endX + 1, startY + selection);
            break;
        case 10:
            aprint("[", endX - 1, startY + selection);
            aprint("]", endX + 2 + !(savefile->settings.placeEffect), startY + selection);
            break;
        case 11:
            if (savefile->settings.shakeAmount > 0)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.shakeAmount < 4)
                aprint(">", endX + shakeString.size(), startY + selection);
            break;
        case 12:
            aprint("[", endX - 1, startY + selection);
            aprint("]", endX + 2 + !(savefile->settings.floatText), startY + selection);
            break;
        case 13:
            if (savefile->settings.maxQueue > 1)
                aprint("<", endX - 1, startY + selection);
            if (savefile->settings.maxQueue < 5)
                aprint(">", endX + 1, startY + selection);
            break;
        case 14:
            aprint("[", 12, 17);
            aprint("]", 17, 17);
            break;
        }
    }

    if (showGame) {
        showQueue();
        showPawn();
        showShadow();
        showHold();
        showBackground(0);

        if(selection == 9){
            for(int i = 0; i < game->lengthX; i++){
                setTile(25, 10 + i, 18, tileBuild(3, false, false, savefile->settings.lightMode));
                setTile(25, 10 + i, 19, tileBuild(3, false, false, savefile->settings.lightMode));
            }
        }
    } else {
        clearSprites(128);
    }

    showSprites(128);

}

void GraphicsScene::deinit(){
    reset();

    // REG_DISPCNT |= DCNT_BG3;
    toggleBG(3,true);
    enableBlend(prevBld);
    // REG_BLDCNT = prevBld;
    buildBG(3,0,27,0,3,false);
    // REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);

    // irq_disable(II_HBLANK);
    gradient(1);

}

void graphicTest() {

}

void setClearEffect(){
    switch(savefile->settings.clearEffect){
    case 0:
        loadTiles(0, 3, sprite4tiles_bin, sprite4tiles_bin_size/32);
        // memcpy16(&tile_mem[0][3], sprite4tiles_bin, sprite4tiles_bin_size / 2);
        break;
    case 1:
        loadTiles(0, 3, sprite25tiles_bin, sprite25tiles_bin_size/32);
        // memcpy16(&tile_mem[0][3], sprite25tiles_bin, sprite25tiles_bin_size / 2);
        break;
    case 2:
        loadTiles(0, 3, sprite26tiles_bin, sprite26tiles_bin_size/32);
        // memcpy16(&tile_mem[0][3], sprite26tiles_bin, sprite26tiles_bin_size / 2);
        break;
    default:
        loadTiles(0, 3, sprite26tiles_bin, sprite26tiles_bin_size/32);
        break;
    }
}

static const std::list<std::string> labels = {
    "Background",
    "Grid",
    "Skin",
    "Ghost Piece",
    "Frame Color",
    "Palette",
    "Block Edges",
    "Light Mode",
    "Board Effects",
    "Clear Style",
    "Place Effect",
    "Screen Shake",
    "Clear Text",
    "Previews",
};

void showLabels(){
    int counter = 0;

    for(auto const& label : labels){
        #ifdef PC
        if(label == "Place Effect"){
            aprintColor(label, startX, startY + counter++, 1);
            continue;
        }

        #endif
        aprint(label,startX, startY + counter++);
    }
}

void gradientEditor(){
    // irq_enable(II_HBLANK);

    // int display_value = REG_DISPCNT;
    // REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG2; //Disable all backgrounds except text
    toggleBG(0, false);
    toggleBG(1, false);
    toggleBG(2, true);
    toggleBG(3, false);

    int color[3];

    for(int i = 0; i < 3; i++){
        color[i] = (savefile->settings.backgroundGradient >> (5*i)) & 0x1f;
    }

    int selection = 0;

    int das = 0;

    const int dasMax = 10;
    const int arrMax = 4;

    int c = 0;

    aprint("Press B to Exit",7,18);

    while(closed()){
        vsync();
        key_poll();
        showSprites(128);

        if(key_hit(KEY_START) || key_hit(KEY_B)){
            sfx(SFX_MENUCANCEL);
            break;
        }

        if(key_hit(KEY_LEFT)){
            if(selection > 0)
                selection--;

            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_RIGHT)){
            if(selection < 2)
                selection++;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_UP)){
            if(color[selection] < 31)
                color[selection]++;
            sfx(SFX_SHIFT2);
        }

        if(key_hit(KEY_DOWN)){
            if(color[selection] > 0)
                color[selection]--;
            sfx(SFX_SHIFT2);
        }

        if(key_is_down(KEY_UP) || key_is_down(KEY_DOWN)){
            das++;

            if(das == dasMax){
                das -= arrMax;

                if(key_is_down(KEY_UP)){
                    if(color[selection] < 31){
                        color[selection]++;
                        sfx(SFX_SHIFT2);
                    }
                }else if(key_is_down(KEY_DOWN)){
                    if(color[selection] > 0){
                        color[selection]--;
                        sfx(SFX_SHIFT2);
                    }
                }
            }
        }else{
            das = 0;
        }

        const int space = 7;
        const int height = 11;

        // u16 * dest = (u16*) &se_mem[29];

        c = 0;

        const int tile = 105;

        for(int i = 0; i < 3; i++){
            int x = 14 + (i-1)*space;

            std::string str = "";
            switch(i){
                case 0: str = "RED"; break;
                case 1: str = "GREEN"; break;
                case 2: str = "BLUE"; break;
            }
            aprint(str,x-str.length()/2 + (i - 1) * (i > 0), height - 4);

            char buff[2];
            posprintf(buff, "%02d",color[i]);

            aprint(buff, x, height);

            if(i == selection){
                setTile(29, x, height - 1 , tileBuild(tile, false, false, 15));
                setTile(29, x+1, height - 1 , tileBuild(tile + 1, false, false, 15));
                setTile(29, x, height + 1 , tileBuild(tile, false, true, 15));
                setTile(29, x+1, height + 1 , tileBuild(tile + 1, false, true, 15));
                // dest[32 * (height - 1) + x] = tile;
                // dest[32 * (height - 1) + x+1] = tile + 1;
                // dest[32 * (height + 1) + x] = tile + 0x800;
                // dest[32 * (height + 1) + x+1] = tile + 0x800 + 1;
            }else{
                aprint("  " , x,height-1);
                aprint("  " , x,height+1);
            }

            c += color[i] << (5*i);
        }

        setGradient(c);
    }

    savefile->settings.backgroundGradient = c;

    //TODO:
    // REG_DISPCNT = display_value;
    toggleBG(0, true);
    toggleBG(1, true);
    toggleBG(2, true);
    clearText();
}

void GradientEditorScene::init(){
    toggleBG(0, false);
    toggleBG(1, false);
    toggleBG(2, true);
    toggleBG(3, false);

    for(int i = 0; i < 3; i++){
        color[i] = (savefile->settings.backgroundGradient >> (5*i)) & 0x1f;
    }

    aprint("Press B to Exit",7,18);

    const int space = 7;
    const int height = 11;

    for(int i = 0; i < 3; i++){
        int x = 14 + (i-1)*space;

        std::string str = "";
        switch(i){
            case 0: str = "RED"; break;
            case 1: str = "GREEN"; break;
            case 2: str = "BLUE"; break;
        }
        aprint(str,x-str.length()/2 + (i - 1) * (i > 0), height - 4);
    }
}

void GradientEditorScene::draw(){

    const int space = 7;
    const int height = 11;

    c = 0;

    const int tile = 105;

    for(int i = 0; i < 3; i++){
        int x = 14 + (i-1)*space;

        char buff[2];
        posprintf(buff, "%02d",color[i]);

        aprint(buff, x, height);

        if(i == selection){
            setTile(29, x, height - 1 , tileBuild(tile, false, false, 15));
            setTile(29, x+1, height - 1 , tileBuild(tile + 1, false, false, 15));
            setTile(29, x, height + 1 , tileBuild(tile, false, true, 15));
            setTile(29, x+1, height + 1 , tileBuild(tile + 1, false, true, 15));
        }else{
            aprint("  " , x,height-1);
            aprint("  " , x,height+1);
        }

        c += color[i] << (5*i);
    }

    showSprites(128);
}

void GradientEditorScene::update(){
    canDraw = 1;
    key_poll();

    setGradient(c);
    gradient(2);

    control();
}

bool GradientEditorScene::control(){
    if(key_hit(KEY_START) || key_hit(KEY_B)){
        sfx(SFX_MENUCANCEL);
        changeScene(previousScene(),Transitions::FADE);
        return true;
    }

    if(key_hit(KEY_LEFT)){
        if(selection > 0)
            selection--;

        sfx(SFX_MENUMOVE);
    }

    if(key_hit(KEY_RIGHT)){
        if(selection < 2)
            selection++;
        sfx(SFX_MENUMOVE);
    }

    if(key_hit(KEY_UP)){
        if(color[selection] < 31)
            color[selection]++;
        sfx(SFX_SHIFT2);
    }

    if(key_hit(KEY_DOWN)){
        if(color[selection] > 0)
            color[selection]--;
        sfx(SFX_SHIFT2);
    }

    if(key_is_down(KEY_UP) || key_is_down(KEY_DOWN)){
        das++;

        if(das == dasMax){
            das -= arrMax;

            if(key_is_down(KEY_UP)){
                if(color[selection] < 31){
                    color[selection]++;
                    sfx(SFX_SHIFT2);
                }
            }else if(key_is_down(KEY_DOWN)){
                if(color[selection] > 0){
                    color[selection]--;
                    sfx(SFX_SHIFT2);
                }
            }
        }
    }else{
        das = 0;
    }

    return false;
}

void GradientEditorScene::deinit(){
    savefile->settings.backgroundGradient = c;

    toggleBG(0, true);
    toggleBG(1, true);
    toggleBG(2, true);
    clearText();
}

void GradientElement::action(){
    sfx(SFX_MENUCONFIRM);
    changeScene(new GradientEditorScene(),Transitions::FADE);
}

void newGraphicsScene::init(){
    loadTiles(2,102,sprite37tiles_bin,sprite37tiles_bin_size/32);
    loadTiles(2,105,sprite41tiles_bin,sprite41tiles_bin_size/32);

    clearSprites(128);

    elementList = getElementList();
    options = (int) elementList.size();

    startY = 24;
    if(options < 7){
       startY += ((7 - options) * 8) & ~0xf;
    }

    selection = 0;

    //transparent gray
    setTiles(27,0,32*32,tileBuild(12, false, false, 4 * savefile->settings.lightMode));

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i] = new WordSprite(i, 32 + i * 3, 256 + i * 12);

    for (int i = 0; i < 7; i++)
        labels[i] = new WordSprite(MAX_WORD_SPRITES + i, 32 + 3 * MAX_WORD_SPRITES + i * 5, 512 + 64 + i * 20, true);

    loadSpriteTiles(255, blockSprite, 1, 1);
    for(int i = 0; i < 2; i++){
        cursorSprites[i] = &obj_buffer[14 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 255, 5, 0);
        sprite_enable_affine(cursorSprites[i], 10, true);
        sprite_hide(cursorSprites[i]);
    }

    //faded Text Palette
    color_fade_palette(13 + 16, 0, &palette[1][14*16], 0, 8, 15);
    color_fade_palette(13, 0, &palette[1][14*16], 0, 8, 15);

    if(name() != ""){
        bool found = false;
        int count = 0;
        auto it = path.begin();
        while(it != path.end()){
            if(*it == name()){
                found = true;
                break;
            }
            count++;
            it++;
        }

        if(found){
            int n = path.size() - count;
            for(int i = 0; i < n; i++)
                path.pop_back();
        }else{
            path.push_back(name());
        }
    }

    int i = 0;
    auto it = path.begin();
    while(it != path.end()){
        if(i != 0)
            p += " > ";

        p += *it;

        it++;
        i++;
    }

    int width = getVariableWidth(p) >> 3;
    pathCount = (width) / 12 + 1;

    int wordIndex = MAX_WORD_SPRITES - pathCount - 1;

    wordSprites[wordIndex]->setText(p);

    for(int i = 0; i < pathCount; i++)
        wordSprites[wordIndex+i]->show(i * 12 * 8 + 4, 4, 14);

    setPalette();
    setSkin();
    setGradient(savefile->settings.backgroundGradient);

    if (savefile->settings.lightMode)
        setPaletteColor(0, 0, 0x5ad6, 1);
        // memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        setPaletteColor(0, 0, 0x0000, 1);
        // memset16(pal_bg_mem, 0x0000, 1);

    clearTilemap(26);
    // memset16(&se_mem[26], 0x0000, 32 * 20);

    delete game;
    game = new Game(DIG,bigMode);
    game->setGoal(0);
    game->setLevel(1);
    game->rotationSystem = SRS;

    game->update();

    game->pawn.y += 1;

    enableBlend((1 << 6) + (0b111011 << 8) + (1 << 3));
    // enableBlend(0);

    buildBG(3, 0, 27, 0, 0, 0);

    drawFrameBackgrounds();

    loadSpriteTiles(512, sprite6tiles_bin,4,4);

    loadSpriteTiles(512 + 16, sprite6tiles_bin,4,3);
    loadSpriteTilesPartial(512 + 16, &sprite6tiles_bin[128], 0, 3,4,1,4);
    loadSpriteTiles(512 + 32, &sprite6tiles_bin[128],4,2);
    loadSpriteTilesPartial(512 + 32, &sprite6tiles_bin[128],0,2,4,2,4);
    loadSpriteTiles(512 + 48, &sprite6tiles_bin[128],4,1);
    loadSpriteTilesPartial(512 + 48, &sprite6tiles_bin[128],0,1,4,3,4);

    draw();

    refreshGame = true;
}

void newGraphicsScene::draw(){
    if(showText){
        if(refreshText){
            setTiles(27, 0, 32 * 32, tileBuild(12, false, false, 4 * savefile->settings.lightMode));
        }

        const int space = 1;
        const int startX = 3 * 8;
        const int endX = 30-4;

        int offset = (sinLut(cursorFloat) * 2) >> 12;
        FIXED scale = float2fx((1.0 - ((float) 0.1 * offset)));

        int verticalOffset = 0;
        if(moving){
            int temp = lerp((movingDirection) * (space + 1) * 8,0,64*movingTimer);
            verticalOffset = lerp(temp,0,64*movingTimer);
            setLayerScroll(2, 0, -verticalOffset);
        }else{
            setLayerScroll(2, 0, 0);
        }

        int i = 0;//drawing index, starts at listStart
        int counter = 0;//element index
        for(auto & option : elementList){
            if(counter < listStart){
                counter++;
                continue;
            }

            if(i > elementMax + 1){ // break if elements drawn reached max
                continue;
            }

            std::string opt = option->getCurrentOption();

            int index = (i + listStart) % (elementMax + 2);

            if(refreshText){
                if(counter == selection)
                    wordSprites[index]->setText(option->getCursor(opt)); //draw option with text cursor
                else
                    wordSprites[index]->setText(" " + opt); // draw option without text cursor

                std::string label = option->getLabel();
                labels[index]->setText(label);
            }

            int x = (endX) * 8 - getVariableWidth(opt);
            int y = startY + 8 * (space + 1) * i + verticalOffset - spriteVOffset;

            wordSprites[index]->setPriority(0);
            wordSprites[index]->show(x, y, 15 - (counter != selection) - (i == elementMax + 1 || (listStart && i == 0)));

            labels[index]->show(startX, y, 15 - ((counter != selection) + (i == elementMax + 1 || (listStart && i == 0))));

            if(counter == selection){ //draw sprite cursor
                for(int i = 0; i < 2; i++){
                    sprite_unhide(cursorSprites[i],0);
                    sprite_enable_affine(cursorSprites[i], 10, true);
                    sprite_set_size(cursorSprites[i], scale, 10);

                    int x = startX - ((8) + offset + 4) - 4;

                    sprite_set_pos(cursorSprites[i], x, y - 5);
                }
            }

            i++;
            counter++;
        }

        for(int j = i; j < elementMax + 2; j++){
            int index = (j + listStart) % (elementMax + 2);

            wordSprites[index]->hide();
            labels[index]->hide();
        }
    }

    if(refreshGame){
        refreshGame = false;

        drawGrid();
        drawFrame(0);

        showQueue();
        showHold();
        showBackground(0);

        for(int i = 0; i < game->lengthX; i++){
            setTile(25, 10 + i, 18, tileBuild(3, false, false, savefile->settings.lightMode));
            setTile(25, 10 + i, 19, tileBuild(3, false, false, savefile->settings.lightMode));
        }
    }

    showPawn();
    showShadow();
    screenShake();

    showPlaceEffect();

    if(savefile->settings.floatText){
        showClearText();
    }

    showSprites(128);

    refreshText = false;
}

void newGraphicsScene::update(){
    canDraw = 1;
    key_poll();
    gradient(1);

    toggleBG(0, true);
    toggleBG(1, true);
    toggleBG(2, true);
    toggleBG(3, true);

    cursorFloat += 6;
    if(cursorFloat >= 512)
        cursorFloat = 0;

    if(moving){
        if(++movingTimer > 4){
            moving = false;
            movingHor = false;
            movingTimer = 0;
            movingDirection = 0;
            refreshText = true;
        }
    }

    control();
}

bool newGraphicsScene::control(){
    // if(key_hit(KEY_A)){
    //     sfx(SFX_MENUCONFIRM);
    //     if(selection < options - 1)
    //         selection++;
    //     // auto it = optionList.begin();
    //     // std::advance(it,selection);

    //     // sceneSwitcher(*it);

    //     // previousElement = "";
    //     return true;
    // }

    if(key_hit(KEY_B)){
        sfx(SFX_MENUCANCEL);
        previousElement = name();
        previousSelection = selection;

        if(path.size())
            path.pop_back();

        changeScene(previousScene(),Transitions::FADE);
        return true;
    }

    if(key_hit(KEY_LEFT)){
        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action(-1);

        refreshText = true;
        refreshGame = true;
    }

    if(key_hit(KEY_RIGHT)){
        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action(1);

        refreshText = true;
        refreshGame = true;
    }

    if(key_hit(KEY_A)){
        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action();

        refreshText = true;
    }

    if(key_hit(KEY_R)){
        showText = !showText;

        if(showText){
            setTiles(27, 0, 32 * 32, tileBuild(12, false, false, 4 * savefile->settings.lightMode));

            int wordIndex = MAX_WORD_SPRITES - pathCount - 1;
            for(int i = 0; i < pathCount; i++)
                wordSprites[wordIndex+i]->show(i * 12 * 8 + 4, 4, 14);

        }else{
            clearTilemap(27);

            for(int j = 0; j < elementMax + 2; j++){
                int index = (j + listStart) % (elementMax + 2);

                wordSprites[index]->hide();
                labels[index]->hide();
            }

            for(int i = 0; i < 2; i++)
                sprite_hide(cursorSprites[i]);

            int wordIndex = MAX_WORD_SPRITES - pathCount - 1;
            for(int i = 0; i < pathCount; i++)
                wordSprites[wordIndex+i]->hide();
        }

        refreshGame = true;
    }

    int prev = listStart;

    if (key_hit(KEY_UP)) {
        movingDirection = -1;

        if (selection == 0)
            selection = options - 1;
        else
            selection--;

        if(selection == options -1)
            listStart = max(0,selection-elementMax);
        else if(selection < listStart + 1){
            listStart--;
            if(listStart < 0)
                listStart = 0;
        }

        sfx(SFX_MENUMOVE);
        refreshText = true;
        // refreshGame = true;
    }

    if (key_hit(KEY_DOWN)) {
        movingDirection = 1;

        if (selection == options - 1)
            selection = 0;
        else
            selection++;

        if(selection == 0)
            listStart = 0;
        else if(selection > listStart + elementMax)
            listStart++;

        sfx(SFX_MENUMOVE);
        refreshText = true;
        // refreshGame = true;
    }

    if (key_is_down(KEY_UP) || key_is_down(KEY_DOWN)) {
        if (dasVer < maxDas) {
            dasVer++;
        } else {
            int dir = (key_is_down(KEY_UP)? -1 : 1);

            if (selection + dir >= 0 && selection + dir <= options-1 && arr++ > maxArr) {
                arr = 0;

                movingDirection = dir;

                selection += dir;

                if(dir < 0){
                    if(selection == options -1)
                        listStart = max(0,selection-elementMax);
                    else if(selection < listStart + 1){
                        listStart--;
                        if(listStart < 0)
                            listStart = 0;
                    }
                }else{
                    if(selection == 0)
                        listStart = 0;
                    else if(selection > listStart + elementMax)
                        listStart++;

                }

                refreshText = true;
                // refreshGame = true;
                sfx(SFX_MENUMOVE);
            }
        }
    } else {
        dasVer = 0;
    }

    if (key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)) {
        if (dasHor < maxDas) {
            dasHor++;
        } else{
            int dir = (key_is_down(KEY_LEFT)? -1 : 1);
            if (arr++ > maxArr) {
                arr = 0;

                auto it = elementList.begin();
                std::advance(it,selection);

                (*it)->action(dir);

                refreshText = true;
                refreshGame = true;
            }
        }
    } else {
        dasHor = 0;
    }

    if(prev != listStart){
        moving = true;
    }

    if(key_hit(KEY_SELECT)){
        setDefaultGraphics(savefile, 0);
        setSkin();
        setPalette();
        setTiles(27, 0, 32 * 32, tileBuild(12, false, false, 4 * savefile->settings.lightMode));
    }

    return false;
}

void newGraphicsScene::deinit(){
    buildBG(3, 0, 27, 0, 1, true);

    clearSprites(128);
    showSprites(128);

    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);
    clearSpriteTiles(2, 100, 1);
    clearSpriteTiles(256, 256, 1);

    clearText();

    auto it = elementList.begin();

    while(it != elementList.end()){
        delete *it;
        it++;
    }

    gradient(false);
    spriteVOffset = 0;
}
