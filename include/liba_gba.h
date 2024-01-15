#pragma once
#include <stdint.h>
#include <tonc.h>
#include "interrupt.h"
#include "mm_types.h"
#include "audio_files.h"
/* #include "soundbank.h" */
#include "posprintf.h"
#include "LinkUniversal.hpp"
#include <map>

static inline void setTile(int sbb, int x, int y, uint32_t value){
    se_mem[sbb][y * 32 + x] = value & 0xffff;
}

static inline uint32_t tileBuild(int index, bool flipx, bool flipy, int palette){
    return 0x1000 * palette + 0x400 * flipx + 0x800 * flipy + index;
}

INLINE u32 tileBuild(u16 value){
    return value;
}

INLINE void setSpritePixel(int index, u8 tx, u8 ty, u8 width, u8 x, u8 y, u8 color){
    TILE * t = &tile_mem[4][index + ty * width + tx];

    if(color)
        t->data[y] |= color << (x * 4);
    else
        t->data[y] &= ~(0xf << (x * 4));
}

INLINE void setSpriteTile(int index, u8 tx, u8 ty, u8 width, const void *src){
    memcpy32(&tile_mem[4][index + ty * width + tx], src, 8);
}

INLINE void clearSpriteTile(int index, u8 tx, u8 ty, u8 width){
    memset32(&tile_mem[4][index+ ty * width + tx],0,8);
}

INLINE void setTileRow(int tileset, int index, int row, u32 data){
    TILE* tile = &tile_mem[tileset][index];
    tile->data[row] = data;
}

INLINE void setTilesetPixel(int cbb, int index, int x, int y, int color){
    TILE* tile = &tile_mem[cbb][index];

    tile->data[y] |= (color) << (x*4);
}

INLINE void color_fade_fast(int palette, int index, COLOR *src, COLOR color, int count, u32 alpha){
    clr_fade_fast(src, color, &pal_bg_mem[palette * 16 + index], count, alpha);
}

INLINE s16 sinLut(int angle){
    return sin_lut[angle];
}

INLINE void setSpriteTileRow(int index, int y, int n){
    TILE* tile = &tile_mem[4][index];

    tile->data[y] |= n;
}

extern OBJ_AFFINE* obj_aff_buffer;

extern mm_word myEventHandler(mm_word msg, mm_word param);

#define blendInfo REG_BLDCNT

extern unsigned __iwram_start__;
extern unsigned __iwram_top;
extern unsigned __fini_array_end;
extern unsigned __ewram_start;
extern unsigned __ewram_end;
extern char __eheap_start[], __eheap_end[];

INLINE int iwramUsage(){
    auto iwram_start = reinterpret_cast<uint8_t*>(&__iwram_start__);
    auto iwram_end = reinterpret_cast<uint8_t*>(&__fini_array_end);

    return iwram_end - iwram_start;
}

INLINE int ewramUsage(){
    auto ewram_start = reinterpret_cast<uint8_t*>(&__ewram_start);
    auto ewram_end = reinterpret_cast<uint8_t*>(&__ewram_end);

    return ewram_end - ewram_start;
}

INLINE int ewramHeapUsage(){
    auto ewram_start = reinterpret_cast<uint8_t*>(&__eheap_start);
    auto ewram_end = reinterpret_cast<uint8_t*>(&__eheap_end);

    return ewram_end - ewram_start;
}

extern u16 gradientTable[192 + 1];

extern std::map<int,std::string> keyToString;

#define UNBOUND 0

extern void toggleSpriteSorting(bool state);
