#include "scene.hpp"
#include "sprites.h"
#include "logging.h"

#include "site_qr_bin.h"

void LinksScene::init(){
    clearSprites(128);

    //backgroundGrid
    setTiles(26,0,32*32, tileBuild(35 * (!savefile->settings.lightMode), false, false, 0));

    setTiles(27,0,32*32,tileBuild(34, false, false, 0));

    for (int i = 0; i < MAX_WORD_SPRITES; i++)
        wordSprites[i] = new WordSprite(i,64 + i * 5, 256 + i * 20, true);

    //faded Text Palette
    color_fade_palette(13 + 16, 0, &palette[1][14*16], 0, 8, 15);

    enableBlend((0b101111 << 8) + (1 << 6) + (1 << 3));

    qr = &obj_buffer[0];

    const int qrTile = 300;

    sprite_unhide(qr,0);
    sprite_set_attr(qr, ShapeSquare, 2, qrTile, 0, 1);
    sprite_enable_affine(qr, 8, true);
    sprite_set_size(qr, 1 << 7, 8);
    sprite_set_pos(qr,0,0);

    u16 * src = (u16 *) site_qr_bin;

    for(int i = 0; i < 29; i++){
        for(int j = 0; j < 29; j++){
            int color = (src[i * 29 + j] != 0)? 7 : 6;
            setSpritePixel(qrTile, j / 8, i / 8, 4, j % 8, i % 8, color);
        }
    }
}

void LinksScene::draw(){
    fallingBlocks();
    toggleBG(3,true);

    // int startX = 30;
    // int counter = 0;
    // int height = 0;
    // int i = 0;
    // int gaps = 0;

    // for(auto option = creditsText.begin(); option != creditsText.end(); ++option){
    //     if(counter < listStart){
    //         counter++;
    //         if(*option == "")
    //             gaps++;

    //         continue;
    //     }

    //     if(*option == ""){
    //         height += space;
    //         continue;
    //     }

    //     if(i >= maxShow)
    //         break;

    //     int index = (i + listStart-gaps) % maxShow;

    //     wordSprites[index]->setText(*option);

    //     int y = height - scrollOffset;

    //     if(y > SCREEN_HEIGHT)
    //         break;

    //     wordSprites[index]->show(startX,height - scrollOffset,15);

    //     i++;
    //     height += space;
    // }

    // for(int j = i; j < maxShow; j++){
    //     int index = (j + listStart - gaps) % maxShow;

    //     wordSprites[index]->hide();
    // }

    showSprites(128);
}

bool LinksScene::control(){

    if(key_hit(KEY_B)){
        sfx(SFX_MENUCANCEL);
        previousElement = name();

        changeScene(previousScene(),Transitions::FADE);
    }

    return false;
}

void LinksScene::update(){
    canDraw = 1;
    key_poll();

    // if(delay < delayMax){
    //     delay++;
    // }else if(++scrollTimer >= scrollTimerMax){
    //     scrollTimer = 0;
    //     if(++scrollOffset >= space){
    //         scrollOffset = 0;
    //         listStart++;
    //     }
    // }

    // if(listStart > 35){
    //     changeScene(previousScene(),Transitions::FADE);
    //     return;
    // }

    control();
}

void LinksScene::deinit(){
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
