
#include "scene.hpp"
#include "sprites.h"
#include "logging.h"

#ifndef MULTIBOOT

std::list<std::string> creditsText = {
    "",
    "",
    "",
    "",
    "Menu Music:",
    "  - veryshorty-extended",
    "        by supernao",
    "",
    "  - optikal innovation",
    "       by substance",
    "",
    "  - space taxi",
    "        by schmid",
    "",
    "",
    "In-Game Music:",
    "  - Thirno",
    "        by Nikku4211",
    "",
    "  - oh my god!",
    "        by kb-zip",
    "",
    "  - unsuspected <h>",
    "        by curt cool",
    "",
    "  - Warning Infected!",
    "        by Basq",
    "",
    "  - __detersive__",
    "        by xtd",
    "",
    "  - schizofrenium_long",
    "        by panther, karl",
};

void CreditsScene::init(){
    clearSprites(128);

    //backgroundGrid
    setTiles(26,0,32*32, tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));

    setTiles(27,0,32*32,tileBuild(34, false, false, 0));

    #ifdef TE
    //letterbox on pc
    buildBG(0, 0, 29, 0, 0, true);
    TILE tile;
    memset32(&tile,0xeeeeeeee,8);
    loadTiles(0, 50, &tile, 1);

    setTiles(29, 20 * 64, 44 * 64,50);
    #endif

    for (int i = 0; i < MAX_WORD_SPRITES; i++){
        wordSprites[i] = new WordSprite(i,64 + i * 5, 256 + i * 20, true);
        wordSprites[i]->setPriority(1);
    }

    enableBlend((0b101111 << 8) + (1 << 6) + (1 << 3));
}

void CreditsScene::draw(){
    fallingBlocks();
    toggleBG(3,true);

    int startX = 30;
    int counter = 0;
    int height = 0;
    int i = 0;
    int gaps = 0;

    for(auto option = creditsText.begin(); option != creditsText.end(); ++option){
        if(counter < listStart){
            counter++;
            if(*option == "")
                gaps++;

            continue;
        }

        if(*option == ""){
            height += space;
            continue;
        }

        if(i >= maxShow)
            break;

        int index = (i + listStart-gaps) % maxShow;

        wordSprites[index]->setText(*option);

        int y = height - scrollOffset;

        if(y > SCREEN_HEIGHT)
            break;

        wordSprites[index]->show(startX,height - scrollOffset,15);

        i++;
        height += space;
    }

    for(int j = i; j < maxShow; j++){
        int index = (j + listStart - gaps) % maxShow;

        wordSprites[index]->hide();
    }

    showSprites(128);
}

bool CreditsScene::control(){

    MenuKeys k = savefile->settings.menuKeys;

    if(key_hit(k.cancel)){
        sfx(SFX_MENUCANCEL);
        previousElement = name();

        changeScene(previousScene(),Transitions::FADE);
    }else if(key_is_down(k.confirm)){
        scrollTimerMax = 1;
    }else if(key_is_down(KEY_FULL)){
        scrollTimerMax = 0xffff;
    }else{
        scrollTimerMax = 4;
    }

    return false;
}

void CreditsScene::update(){
    canDraw = 1;
    key_poll();

    if(delay < delayMax){
        delay++;
    }else if(++scrollTimer >= scrollTimerMax){
        scrollTimer = 0;
        if(++scrollOffset >= space){
            scrollOffset = 0;
            listStart++;
        }
    }

    if(listStart > 35){
        changeScene(previousScene(),Transitions::FADE);
        return;
    }

    control();
}

void CreditsScene::deinit(){
    clearSprites(128);
    showSprites(128);

    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);
    clearSpriteTiles(2, 100, 1);
    clearSpriteTiles(256, 256, 1);

    clearText();

    for(int i = 0; i < MAX_WORD_SPRITES; i++)
        delete wordSprites[i];

    buildBG(0, 2, 29, 0, 0, true);
}

#endif
