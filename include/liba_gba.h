#pragma once
#include <stdint.h>
#include <tonc.h>
#include "mm_types.h"

static inline void setTile(int sbb, int x, int y, uint32_t value){
    se_mem[sbb][y * 32 + x] = value & 0xffff;
}

static inline uint32_t tileBuild(int index, bool flipx, bool flipy, int palette){
    return 0x1000 * palette + 0x400 * flipx + 0x800 * flipy + index;
}

INLINE void setSpritePixel(int index, u8 tx, u8 ty, u8 width, u8 x, u8 y, u8 color){
    TILE * t = &tile_mem[4][index + ty * width + tx];

    t->data[y] |= color << (x);
}

INLINE void setSpriteTile(int index, u8 tx, u8 ty, u8 width, const void *src){
    memcpy32(&tile_mem[4][index + ty * 4 + tx], src, 8);
}

INLINE void clearSpriteTile(int index, u8 tx, u8 ty, u8 width){
    memset32(&tile_mem[4][index+ ty * width + tx],0,8);
}

extern OBJ_AFFINE* obj_aff_buffer;

extern mm_word myEventHandler(mm_word msg, mm_word param);
