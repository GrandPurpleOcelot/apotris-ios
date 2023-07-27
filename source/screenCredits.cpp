
#include "scene.hpp"
#include "sprites.h"
#include "logging.h"

std::string creditsText = "\
Menu Music:\n\
  - veryshorty-extended\n\
        by supernao\n\
  - optikal innovation\n\
        by substance\n\
  - space taxi\n\
        by schmid\n\
\n\
In-Game Music:\n\
  - Thirno\n\
        by Nikku4211\n\
  - oh my god!\n\
        by kb-zip\n\
  - unsespected <h>\n\
        by curt cool\n\
  - Warning Infected!\n\
        by Basq\n\
  - __detersive__\n\
        by xtd\n\
  - schizofrenium_long\n\
        by panther, karl\n\
";

void CreditsScene::init(){
    clearSprites(128);

    //backgroundGrid
    setTiles(26,0,32*32, tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));

    setTiles(27,0,32*32,tileBuild(34, false, false, 0));

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i] = new WordSprite(i,64 + i * 5, 256 + i * 20);

    //faded Text Palette
    color_fade_palette(13 + 16, 0, &palette[1][14*16], 0, 8, 15);

    enableBlend((0b101111 << 8) + (1 << 6) + (1 << 3));

    int startX = 30;
    int startY = 20;

    int space = 12;
    int counter = 0;
}

void CreditsScene::draw(){
    fallingBlocks();
    toggleBG(3,true);

    showSprites(128);
}

bool CreditsScene::control(){

    if(key_hit(KEY_FULL)){
        sfx(SFX_MENUCANCEL);
        previousElement = name();

        changeScene(previousScene(),Transitions::FADE);
    }

    return false;
}

void CreditsScene::update(){
    canDraw = 1;
    key_poll();

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
}
