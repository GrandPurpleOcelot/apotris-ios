#pragma once

#include <list>
#include "platform.hpp"
#include <string>
#include "font1tiles_bin.h"

#define MAX_WORD_SPRITES 15

class WordSprite {
public:
    std::string text = "";
    int startIndex;
    int startTiles;
    int id;
    int priority = 0;
    bool blend = false;
    OBJ_ATTR* sprites[3];

    void show(int x, int y, int palette) {
        for (int i = 0; i < 3; i++) {
            sprite_unhide(sprites[i], 0);
            sprite_set_attr(sprites[i], ShapeWide, 1, startTiles+i*4, palette, priority);

            if(blend)
                sprite_enable_blend(sprites[i]);

            sprite_set_pos(sprites[i], x + i * 32, y);
        }
    }

    void show(int x, int y, int palette, FIXED scale) {
        for (int i = 0; i < 3; i++) {
            int affId = id*3+i;
            sprite_unhide(sprites[i], 0);
            sprite_set_attr(sprites[i], ShapeWide, 1, startTiles+i*4, palette, priority);

            if(blend)
                sprite_enable_blend(sprites[i]);

            sprite_set_pos(sprites[i], x + fx2int(fxdiv(int2fx( i * 32 ),scale)), y);
            sprite_set_size(sprites[i], scale, affId);
            sprite_enable_affine(sprites[i], affId, true);
        }
    }

    void setPriority(int newPriority){
        priority = newPriority;
    }

    void hide() {
        for (int i = 0; i < 3; i++)
            sprite_hide(sprites[i]);
    }

    void enableBlend(bool state){
        blend = state;
    }

    void setText(std::string _text) {
        if (_text == text)
            return;

        text = _text;


        if(text == ""){
            clearSpriteTiles(startTiles, 12, 0);
            // memset32(&tile_mem[4][startTiles], 0, 12 * 8);
            return;
        }

        // clearSpriteTiles(startTiles, 12, 1);

        // naprintSprite(text,startTiles);

        int n = min((int)text.size(),12);

        TILE* font = (TILE*)font1tiles_bin;
        int i;
        for (i = 0; i < n; i++) {
            int c = text[i] - 32 - 1;

            if(c >= 0)
                setSpriteTile(startTiles,i,0,0,&font[c]);
            else
                clearSpriteTiles(startTiles + i, 1, 1);
            // memcpy32(&tile_mem[4][startTiles + i], &font[c], 8);
            // dma3_cpy(&tile_mem[4][startTiles + i], &font[c], 32);
        }

        if(i < 12){

            //TODO:
            //"+ i" might be wrong
            clearSpriteTiles(startTiles + i, 12-i, 1);
            // memset32(&tile_mem[4][startTiles + i], 0, (12-i) * 8);
        }
    }

    WordSprite(int _id,int _index, int _tiles) {
        id = _id;
        startIndex = _index;
        startTiles = _tiles;

        for (int i = 0; i < 3; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }
};

