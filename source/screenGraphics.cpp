#include "def.h"
#include "platform.hpp"
#include "text.h"
#include "sprites.h"
#include <string>
#include "posprintf.h"
#include "logging.h"
#include "scene.hpp"
#include "sceneGraphics.hpp"

using namespace BlockEngine;

void setClearEffect(){
    switch(savefile->settings.clearEffect){
    case 0:
        loadTiles(0, 3, sprite4tiles_bin, sprite4tiles_bin_size/32);
        break;
    case 1:
        loadTiles(0, 3, sprite25tiles_bin, sprite25tiles_bin_size/32);
        break;
    case 2:
        loadTiles(0, 3, sprite26tiles_bin, sprite26tiles_bin_size/32);
        break;
    default:
        loadTiles(0, 3, sprite26tiles_bin, sprite26tiles_bin_size/32);
        break;
    }
}

void GradientEditorScene::init(){
    toggleBG(0, false);
    toggleBG(1, false);
    toggleBG(2, true);
    toggleBG(3, false);

    for(int i = 0; i < 3; i++){
        color[i] = (savefile->settings.backgroundGradient >> (5*i)) & 0x1f;
    }

    for(int i = 0; i < 3; i++){
        color2[i] = (savefile->settings.backgroundGradient >> (5*i + 16)) & 0x1f;
    }

    clearSpriteTiles(256, 12 * MAX_WORD_SPRITES, 1);
    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i] = new WordSprite(i, 32 + i * 3, 256 + i * 12);

    resetSmallText();
    drawText();

    setMaxSelection();

    loadSpriteTiles(254, &sprite48tiles_bin, 1, 1);

    loadSpriteTiles(255, blockSprite, 1, 1);
    cursorSprite = &obj_buffer[14];
    sprite_set_attr(cursorSprite, ShapeSquare, 0, 255, 5, 0);
    sprite_enable_affine(cursorSprite, 10, true);
    sprite_hide(cursorSprite);

    for(int i = 0; i < 6; i++){
        colorSprites[i] = &obj_buffer[15 + i];

        sprite_set_attr(colorSprites[i], ShapeSquare, 0, 254, i % 3, 0);
        sprite_hide(colorSprites[i]);
    }

    setPaletteColor(16 + 0, 15, RGB15(31, 0, 0), 1);
    setPaletteColor(16 + 1, 15, RGB15(0, 31, 0), 1);
    setPaletteColor(16 + 2, 15, RGB15(5, 5, 31), 1);
}

void GradientEditorScene::draw(){

    const int space = 4;
    const int height = 11;

    c = 0;

    const int tile = 105;

    if(savefile->settings.backgroundType == 0){
        for(int i = 0; i < 3; i++){
            int x = 19 + (i-1)*space;

            char buff[8];
            posprintf(buff, "%02d",color[i]);

            aprintColor(buff, x, height, !onValues);

            if(i == colorSelection && onValues){
                setTile(29, x, height - 1 , tileBuild(tile, false, false, 15));
                setTile(29, x+1, height - 1 , tileBuild(tile + 1, false, false, 15));
                setTile(29, x, height + 1 , tileBuild(tile, false, true, 15));
                setTile(29, x+1, height + 1 , tileBuild(tile + 1, false, true, 15));
            }else{
                aprint("  " , x,height-1);
                aprint("  " , x,height+1);
            }

            c += color[i] << (5*i);

            sprite_unhide(colorSprites[i],0);
            sprite_set_pos(colorSprites[i], x * 8 - 9, height * 8);
        }

        for(int i = 0; i < 3; i++)
            sprite_hide(colorSprites[i+3]);

    }else if(savefile->settings.backgroundType == 1){
        int height = 8;

        for(int i = 0; i < 3; i++){
            int x = 19 + (i-1)*space;

            char buff[8];
            posprintf(buff, "%02d",color[i]);

            aprintColor(buff, x, height, !(selection == 1 && onValues));

            if(i == colorSelection && onValues && selection == 1){
                setTile(29, x, height - 1 , tileBuild(tile, false, false, 15));
                setTile(29, x+1, height - 1 , tileBuild(tile + 1, false, false, 15));
                setTile(29, x, height + 1 , tileBuild(tile, false, true, 15));
                setTile(29, x+1, height + 1 , tileBuild(tile + 1, false, true, 15));
            }else{
                aprint("  " , x,height-1);
                aprint("  " , x,height+1);
            }

            c += color[i] << (5*i);

            sprite_unhide(colorSprites[i],0);
            sprite_set_pos(colorSprites[i], x * 8 - 9, height * 8);
        }

        height = 13;

        for(int i = 0; i < 3; i++){
            int x = 19 + (i-1)*space;

            char buff[8];
            posprintf(buff, "%02d",color2[i]);

            aprintColor(buff, x, height, !(selection == 2 && onValues));

            if(i == colorSelection && onValues && selection == 2){
                setTile(29, x, height - 1 , tileBuild(tile, false, false, 15));
                setTile(29, x+1, height - 1 , tileBuild(tile + 1, false, false, 15));
                setTile(29, x, height + 1 , tileBuild(tile, false, true, 15));
                setTile(29, x+1, height + 1 , tileBuild(tile + 1, false, true, 15));
            }else{
                aprint("  " , x,height-1);
                aprint("  " , x,height+1);
            }

            c += color2[i] << (5*i + 16);

            sprite_unhide(colorSprites[i + 3],0);
            sprite_set_pos(colorSprites[i + 3], x * 8 - 9, height * 8);
        }

    }

    std::string temp;

    switch(savefile->settings.backgroundType){
    case 0: temp = "Single Color"; break;
    case 1: temp = "Dual Color"; break;
    default: temp = "Single Color"; break;
    }

    std::string typeString;
    if(selection == 0){
        if(savefile->settings.backgroundType > 0)
            typeString += "<";
        else
            typeString += " ";

        typeString += temp;

        if(savefile->settings.backgroundType < 1)
            typeString += ">";
        else
            typeString += " ";
    }else{
        typeString += " " + temp + " ";
    }

    wordSprites[0]->setText(typeString);
    wordSprites[0]->show(25 * 8 - wordSprites[0]->width, 3 * 8, 15);

    showSprites(128);

    int offset = (sinLut(cursorFloat) * 2) >> 12;
    FIXED scale = float2fx((1.0 - ((float) 0.1 * offset)));

    sprite_unhide(cursorSprite,0);
    sprite_enable_affine(cursorSprite, 10, true);
    sprite_set_size(cursorSprite, scale, 10);

    if(onValues){

    }else if(selection == 0){

        int x = 3 * 8 - ((8) + offset + 4) - 4;

        sprite_set_pos(cursorSprite, x, 3 * 8 - 5);
    }else{
        int x = 3 * 8 - ((8) + offset + 4) - 4;

        if(savefile->settings.backgroundType == 0){
            sprite_set_pos(cursorSprite, x, 11 * 8 - 5);
        }else if(savefile->settings.backgroundType == 1){
            if(selection == 1){
                sprite_set_pos(cursorSprite, x, 8 * 8 - 5);
            }else if(selection == 2){
                sprite_set_pos(cursorSprite, x, 13 * 8 - 5);
            }
        }
    }
}

void GradientEditorScene::drawText(){
    clearText();

    std::string s = "Press START to Exit";
    naprintColor(s, 15 * 8 - (getVariableWidth(s) / 2), 18 * 8, 14);

    naprint("Type:", 3 * 8, 3 * 8);

    if(savefile->settings.backgroundType == 0){
        naprint("Color:", 3 * 8, 11 * 8);
    }else if(savefile->settings.backgroundType == 1){
        naprint("Top:", 3 * 8, 8 * 8);
        naprint("Bottom:", 3 * 8, 13 * 8);
    }
}

void GradientEditorScene::update(){
    canDraw = 1;
    key_poll();

    setGradient(c);
    gradient(2);

    cursorFloat += 6;
    if(cursorFloat >= 512)
        cursorFloat = 0;

    control();
}

bool GradientEditorScene::control(){
    if(key_hit(KEY_START)){
        sfx(SFX_MENUCANCEL);
        changeScene(previousScene(),Transitions::FADE);
        return true;
    }

    if(!onValues){
        if(key_hit(KEY_UP)){
            if(selection > 0)
                selection--;

            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_DOWN)){
            if(selection < maxSelection - 1)
                selection++;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_B)){
            sfx(SFX_MENUCANCEL);
            changeScene(previousScene(),Transitions::FADE);
            return true;
        }

        if(selection == 0){
            if(key_hit(KEY_LEFT)){
                if(savefile->settings.backgroundType > 0){
                    savefile->settings.backgroundType--;
                    drawText();
                    setMaxSelection();
                }

                sfx(SFX_MENUMOVE);
            }

            if(key_hit(KEY_RIGHT)){
                if(savefile->settings.backgroundType < 1){
                    savefile->settings.backgroundType++;
                    drawText();
                    setMaxSelection();
                }

                sfx(SFX_MENUMOVE);
            }
        }else{
            if(key_hit(KEY_RIGHT) || key_hit(KEY_A)){
                onValues = true;
                colorSelection = 0;
                sfx(SFX_MENUCONFIRM);
            }
        }
    }else{
        if(key_hit(KEY_LEFT)){
            if(colorSelection > 0){
                colorSelection--;
                sfx(SFX_MENUMOVE);
            } else{
                onValues = false;
                sfx(SFX_MENUCANCEL);
            }
        }

        if(key_hit(KEY_B)){
            onValues = false;
            sfx(SFX_MENUCANCEL);
        }

        if(key_hit(KEY_RIGHT)){
            if(colorSelection< 2)
                colorSelection++;
            sfx(SFX_MENUMOVE);
        }

        int * arr;
        if(selection == 1)
            arr = (int *) color;
        else
            arr = (int *) color2;

        if(key_hit(KEY_UP)){
            if(arr[colorSelection] < 31)
                arr[colorSelection]++;
            sfx(SFX_SHIFT2);
        }

        if(key_hit(KEY_DOWN)){
            if(arr[colorSelection] > 0)
                arr[colorSelection]--;
            sfx(SFX_SHIFT2);
        }

        if(key_is_down(KEY_UP) || key_is_down(KEY_DOWN)){
            das++;

            if(das == dasMax){
                das -= arrMax;

                if(key_is_down(KEY_UP)){
                    if(arr[colorSelection] < 31){
                        arr[colorSelection]++;
                        sfx(SFX_SHIFT2);
                    }
                }else if(key_is_down(KEY_DOWN)){
                    if(arr[colorSelection] > 0){
                        arr[colorSelection]--;
                        sfx(SFX_SHIFT2);
                    }
                }
            }
        }else{
            das = 0;
        }
    }

    return false;
}

void GradientEditorScene::deinit(){
    savefile->settings.backgroundGradient = c;

    toggleBG(0, true);
    toggleBG(1, true);
    toggleBG(2, true);
    clearText();

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        delete wordSprites[i];
}

void GradientElement::action(){
    sfx(SFX_MENUCONFIRM);
    changeScene(new GradientEditorScene(),Transitions::FADE);
}

void GraphicsScene::init(){
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
    setTiles(27,0,32*32,tileBuild(34, false, false, 0));

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i] = new WordSprite(i, 32 + i * 3, 256 + i * 12);

    for (int i = 0; i < 7; i++)
        labels[i] = new WordSprite(MAX_WORD_SPRITES + i, 32 + 3 * MAX_WORD_SPRITES + i * 5, 512 + 64 + i * 20, true);

    for (int i = 0; i < 3; i++)
        scrollingText[i] = new WordSprite(MAX_WORD_SPRITES + 7 + i, 23 + i * 3, 672 + 64 + i * 12);

    loadSpriteTiles(255, blockSprite, 1, 1);
    for(int i = 0; i < 2; i++){
        cursorSprites[i] = &obj_buffer[14 + i];
        sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 255, 5, 0);
        sprite_enable_affine(cursorSprites[i], 10, true);
        sprite_hide(cursorSprites[i]);
    }

    loadSpriteTiles(910, sprite41tiles_bin, 2, 1);
    for(int i = 0; i < 2; i++){
        arrowSprites[i] = &obj_buffer[21 + i];
        sprite_set_attr(arrowSprites[i], ShapeWide, 0, 910, 14, 0);

        if(i) sprite_enable_flip(arrowSprites[1], false, true);

        sprite_hide(arrowSprites[i]);
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
        wordSprites[wordIndex+i]->show(i * 12 * 8 + 4 + 13 * savefile->settings.aspectRatio, 4, 14);

    setPalette();
    setSkin();
    setGradient(savefile->settings.backgroundGradient);

    if (savefile->settings.lightMode)
        setPaletteColor(0, 0, 0x5ad6, 1);
    else
        setPaletteColor(0, 0, 0x0000, 1);

    clearTilemap(26);

    enableFumen = true;
    fumenString = "v115@fgY4Aeo0Ae4wAeItAeYpAeolAe4hKeAgHfg4DAeoHA?eYLAeIPAe4SAeoWAeQaQ4WaKeAAAchAAA8QeAAA";

    if(game != nullptr)
        delete game;

    game = new Game(NO_MODE,bigMode);

    game->demoGarbage();
    game->setGoal(0);
    game->setLevel(1);
    game->rotationSystem = SRS;

    game->update();

    game->pawn.y += 1;

    enableFumen = false;

    enableBlend((1 << 6) + (0b111011 << 8) + (0b1010));

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

    for(int i = 0; i < 3; i++)
        scrollingText[i]->show(12 * 8 * i,160-8,15);

    scrollingText[0]->setTextSlow(scrollText);
}

void GraphicsScene::draw(){
    if(showText){
        if(refreshText){
            setTiles(27, 0, 32 * 32, tileBuild(34, false, false, 0));
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

            if(refreshText || refreshOption){
                if(counter == selection)
                    wordSprites[index]->setText(option->getCursor(opt)); //draw option with text cursor
                else
                    wordSprites[index]->setText(" " + opt); // draw option without text cursor
            }

            if(refreshText)
                labels[index]->setText(option->getLabel());

            int x = (endX) * 8 - getVariableWidth(opt);
            int y = startY + 8 * (space + 1) * i + verticalOffset - spriteVOffset;

            wordSprites[index]->setPriority(0);
            wordSprites[index]->show(x, y, 15 - (counter != selection) - (i == elementMax + 1 || (listStart && i == 0)));

            labels[index]->show(startX, y, 15 - ((counter != selection) + (i == elementMax + 1 || (listStart && i == 0))));

            if(counter == selection){ //draw sprite cursor
                currentOption = option->getLabel();
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
    drawGrid();

    showPlaceEffect();

    if(savefile->settings.floatText){
        showClearText();
    }

    if(listStart > 0 && showText){
        sprite_unhide(arrowSprites[0], 0);
        sprite_set_pos(arrowSprites[0], 240 / 2 - 8, startY - 11);
    }else{
        sprite_hide(arrowSprites[0]);
    }

    if(listStart + elementMax + 2 <  (int) elementList.size() && showText){
        sprite_unhide(arrowSprites[1], 0);
        sprite_set_pos(arrowSprites[1], 240 / 2 - 8, (elementMax + 2) * 2 * 8 + startY + 0);
    }else{
        sprite_hide(arrowSprites[1]);
    }

    const int width = (savefile->settings.aspectRatio == 0)? 240 : 214;

    if(refreshText){
        scrollText = getDescription(currentOption);
        clearSpriteTiles(672 + 64, 144, 1);
        if(scrollText != ""){
            scrollTextLength = getVariableWidth(scrollText);

            endDelay = 0;
            scrollOffset = - max((width - scrollTextLength),0) / 2;
            scrollDelay = 0;
            scrollingText[0]->setTextSlow(scrollText);
        }else{
            scrollingText[0]->setTextSlow(scrollText);
        }
    }

    int wordIndex = MAX_WORD_SPRITES - pathCount - 1;
    for(int i = 0; i < pathCount; i++)
        wordSprites[wordIndex+i]->show(i * 12 * 8 + 4 + 13 * savefile->settings.aspectRatio, 4, 14);

    showSprites(128);

    refreshText = false;
    refreshOption = false;
}

void GraphicsScene::update(){
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

    const int width = (savefile->settings.aspectRatio == 0)? 240 : 214;

    if(endDelay){
        endDelay--;
        if(!endDelay){
            scrollOffset = 0;
            scrollDelay = 0;
            clearSpriteTiles(672 + 64, scrollTextLength / 8 , 1);
            scrollingText[0]->setTextSlow(scrollText);
        }
    }else if(scrollDelay < scrollDelayMax){
        scrollDelay++;
    }else if(scrollTextLength > width){
        if(++scrollTimer >= scrollTimerMax){
            scrollTimer = 0;
            if(++scrollOffset >= max(scrollTextLength-width,0)){
                endDelay = endDelayMax;
            }
        }
    }

    for(int i = 0; i < 3; i++){
        scrollingText[i]->setScroll(scrollOffset);
        scrollingText[i]->show(12 * 8 * i + 13 * savefile->settings.aspectRatio,160-8,14);
    }

    control();
}

bool GraphicsScene::control(){
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

        refreshGame = true;
        refreshOption = true;
    }

    if(key_hit(KEY_RIGHT)){
        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action(1);

        refreshGame = true;
        refreshOption = true;
    }

    if(key_hit(KEY_A)){
        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action();

        refreshText = true;
        refreshGame = true;
        refreshOption = true;
    }

    if(key_hit(KEY_R)){
        showText = !showText;

        if(showText){
            setTiles(27, 0, 32 * 32, tileBuild(34, false, false, 0));

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

                refreshGame = true;
                refreshOption = true;
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
        drawFrameBackgrounds();

        refreshGame = true;

        sfx(SFX_MENUCONFIRM);
    }

    return false;
}

void GraphicsScene::deinit(){
    buildBG(3, 0, 27, 0, 1, true);

    clearSprites(128);
    showSprites(128);

    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);
    clearSpriteTiles(2, 100, 1);
    clearSpriteTiles(256, 256, 1);
    for(int i = 0; i < 7; i++)
        clearSpriteTiles(512 + 64 + i * 20, 20, 1);

    clearText();

    auto it = elementList.begin();

    while(it != elementList.end()){
        delete *it;
        it++;
    }

    gradient(false);
    spriteVOffset = 0;

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        delete wordSprites[i];

    for (int i = 0; i < 7; i++)
        delete labels[i];

    for (int i = 0; i < 3; i++)
        delete scrollingText[i];
}
