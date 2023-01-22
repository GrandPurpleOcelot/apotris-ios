#pragma once
#include <string>
#include <iostream>
#include "Tilengine.h"
#include <cstring>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

typedef unsigned char  u8;
typedef unsigned short u16, COLOR;
typedef unsigned int   u32;
typedef unsigned long long u64;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;
typedef signed long long s64;

#define KEY_A			0x0001	//!< Button A
#define KEY_B			0x0002	//!< Button B
#define KEY_SELECT		0x0004	//!< Select button
#define KEY_START		0x0008	//!< Start button
#define KEY_RIGHT		0x0010	//!< Right D-pad
#define KEY_LEFT		0x0020	//!< Left D-pad
#define KEY_UP			0x0040	//!< Up D-pad
#define KEY_DOWN		0x0080	//!< Down D-pad
#define KEY_R			0x0100	//!< Shoulder R
#define KEY_L			0x0200	//!< Shoulder L
#define KEY_FULL	0xFFFFFFFF		//!< Define for checking all keys.

typedef struct TILE{
    u32 data[8];
}TILE;

class SpriteAttributes{
public:
    int id;
    int tile_start;
    int x;
    int y;
    int sx;
    int sy;
    int palette;
    int priority;
};

typedef SpriteAttributes OBJ_ATTR;

#define INLINE static inline

typedef int FIXED;
#define FIXED_BITS	8
#define float2fx(f)	(FIXED)(f*(1 << FIXED_BITS))
#define int2fx(i)		((int)(i) << FIXED_BITS)
#define fx2int(f)		((int)(f) >> FIXED_BITS)
#define fx2float(f)	(float)(f)/(1 << FIXED_BITS)

INLINE FIXED fxdiv(FIXED fa, FIXED fb)
{	return ((fa)*(1<<FIXED_BITS))/(fb);			}

#define ALIGN(n)	__attribute__((aligned(n)))

static inline void log(std::string str){
    std::cout << str << "\n";
}

extern TLN_Palette palettes[32];

extern TLN_Tileset tilesets[4];

extern TLN_Tilemap tilemaps[32];

extern TLN_SpriteData spriteData[128];

extern TLN_Spriteset spriteset;

extern void key_poll();

// extern bool canDraw;

INLINE void setTile(int sbb, int x, int y, u32 value){

    TLN_Tile t = new Tile();
    t->value = value;
    // t->index = 1;
    // t->flags = 0;

    TLN_SetTilemapTile(tilemaps[sbb], y, x, t);
}

INLINE u32 tileBuild(int index, bool flipx, bool flipy, int palette){
    TLN_Tile t = new Tile();
    t->index = index & 0xffff;
    t->palette = palette & 0xf;
    t->flipx = flipx;
    t->flipy = flipy;

    u32 result = t->value;

    delete t;

    return result;
}

INLINE void memcpy16(void * dst, const void * src, int count){
    memcpy(dst,src,count*2);
}

INLINE void memcpy32(void * dst, const void * src, int count){
    memcpy(dst,src,count*4);
}

INLINE int qran(){
    return rand();
}

INLINE void sqran(int seed){
    srand(seed);
}

extern u16 __key_curr, __key_prev;

//! \name Basic synchonous keystates
//\{
void key_poll();
INLINE u32 key_curr_state(void);
INLINE u32 key_prev_state(void);

INLINE u32 key_is_down(u32 key);
INLINE u32 key_is_up(u32 key);

INLINE u32 key_was_down(u32 key);
INLINE u32 key_was_up(u32 key);
//\}

//! \name Transitional keystates
//\{
INLINE u32 key_transit(u32 key);
INLINE u32 key_held(u32 key);
INLINE u32 key_hit(u32 key);
INLINE u32 key_released(u32 key);
//\}

//! Get current keystate
INLINE u32 key_curr_state(void)		{	return __key_curr;			}

//! Get previous key state
INLINE u32 key_prev_state(void)		{	return __key_prev;			}

//! Gives the keys of \a key that are currently down
INLINE u32 key_is_down(u32 key)		{	return  __key_curr & key;	}

//! Gives the keys of \a key that are currently up
INLINE u32 key_is_up(u32 key)		{	return ~__key_curr & key;	}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_down(u32 key)	{	return  __key_prev & key;	}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_up(u32 key)		{	return ~__key_prev & key;	}

//! Gives the keys of \a key that are different from before
INLINE u32 key_transit(u32 key)
{	return ( __key_curr ^  __key_prev) & key;	}

//! Gives the keys of \a key that are being held down
INLINE u32 key_held(u32 key)
{	return ( __key_curr &  __key_prev) & key;	}

//! Gives the keys of \a key that are pressed (down now but not before)
INLINE u32 key_hit(u32 key)
{	return ( __key_curr &~ __key_prev) & key;	}

//! Gives the keys of \a key that are being released
INLINE u32 key_released(u32 key)
{	return (~__key_curr &  __key_prev) & key;	}

INLINE COLOR RGB15(int red, int green, int blue)
{	return red + (green<<5) + (blue<<10);					}

INLINE COLOR RGB15_SAFE(int red, int green, int blue)
{	return (red&31) + ((green&31)<<5) + ((blue&31)<<10);	}


INLINE int min(int a, int b){
    return (a < b)? a : b;
}

INLINE void setSpriteTile(int index, u8 tx, u8 ty, u8 width, const void *src){
    // memcpy32(&tile_mem[4][index + ty * 4 + tx], src, 8);
}

//TODO:
INLINE void setSpritePixel(int index, u8 tx, u8 ty, u8 width, u8 x, u8 y, u8 color){
}
