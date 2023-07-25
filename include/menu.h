#pragma once

#include <list>
#include "platform.hpp"
#include <string>
#include "font1tiles_bin.h"
#include "text.h"

#define MAX_WORD_SPRITES 12

class WordSprite {
public:
    std::string text = "";
    int startIndex;
    int startTiles;
    int id;
    int priority = 0;
    bool blend = false;
    OBJ_ATTR* sprites[5];
    int width = 0;
    bool big = false;

    void show(int x, int y, int palette) {
        for (int i = 0; i < 3 * (1+big) - big; i++) {
            sprite_unhide(sprites[i], 0);
            sprite_set_attr(sprites[i], ShapeWide, 1, startTiles+i*4, palette, priority);

            if(blend)
                sprite_enable_blend(sprites[i]);

            sprite_set_pos(sprites[i], x + i * 32, y);
        }
    }

    void show(int x, int y, int palette, FIXED scale) {
        for (int i = 0; i < 3 * (1+big) - big; i++) {
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
        for (int i = 0; i < 3 * (1+big) - big; i++)
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
            clearSpriteTiles(startTiles, (big)?20:12 , 0);
            // memset32(&tile_mem[4][startTiles], 0, 12 * 8);
            return;
        }

        width = getVariableWidth(text);

        clearSpriteTiles(startTiles, (big)?20:12, 1);

        naprintSprite(text,startTiles);
    }

    WordSprite(int _id,int _index, int _tiles) {
        id = _id;
        startIndex = _index;
        startTiles = _tiles;

        for (int i = 0; i < 3; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }

    WordSprite(int _id,int _index, int _tiles, bool _big) {
        id = _id;
        startIndex = _index;
        startTiles = _tiles;
        big = _big;

        for (int i = 0; i < 3 * (1 + big) - big; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }
};

