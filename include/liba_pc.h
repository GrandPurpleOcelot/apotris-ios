#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>

#define TLN_EXCLUDE_WINDOW // needs to be before "Tilengine.h"

#include "Tilengine.h"
#include <cstring>
#include "audio_files.h"
#include <SFML/Window.hpp>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

typedef unsigned char  u8;
typedef unsigned short u16, COLOR;
typedef unsigned int   u32, uint;
typedef unsigned long long u64;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;
typedef signed long long s64;

#define IWRAM_CODE

// #define KEY_A			0x0001	//!< Button A
// #define KEY_B			0x0002	//!< Button B
// #define KEY_SELECT		0x0004	//!< Select button
// #define KEY_START		0x0008	//!< Start button
// #define KEY_RIGHT		0x0010	//!< Right D-pad
// #define KEY_LEFT		0x0020	//!< Left D-pad
// #define KEY_UP			0x0040	//!< Up D-pad
// #define KEY_DOWN		0x0080	//!< Down D-pad
// #define KEY_R			0x0100	//!< Shoulder R
// #define KEY_L			0x0200	//!< Shoulder L
// #define KEY_FULL	0xFFFFFFFF		//!< Define for checking all keys.

#define KEY_A sf::Keyboard::I
#define KEY_B sf::Keyboard::SemiColon
#define KEY_SELECT sf::Keyboard::Backspace
#define KEY_START sf::Keyboard::Enter
#define KEY_RIGHT sf::Keyboard::L
#define KEY_LEFT sf::Keyboard::J
#define KEY_UP sf::Keyboard::C
#define KEY_DOWN sf::Keyboard::K
#define KEY_R sf::Keyboard::Z
#define KEY_L sf::Keyboard::S
#define KEY_FULL	0xFFFFFFFF

#define ATTR0_AFF 0
#define ATTR0_AFF_DBL 0

#define CLR_MASK			0x001F

#define RED_MASK			0x001F
#define RED_SHIFT				 0
#define GREEN_MASK			0x03E0
#define GREEN_SHIFT				 5
#define BLUE_MASK			0x7C00
#define BLUE_SHIFT				10

extern int blendInfo;

typedef struct TILE{
    u32 data[8];
}TILE;

static int spriteIdCounter = 0;

class SpriteAttributes{
public:
    int id = spriteIdCounter++;
    bool enabled = false;
    int tile_start = 0;
    int x = 0;
    int y = 0;
    int sx = 0;
    int sy = 0;
    bool affine = false;
    bool mosaic = false;
    float scalex = 0.0f;
    float scaley = 0.0f;
    int palette = 0;
    int priority = 0;
    bool flipx = false;
    bool flipy = false;
    bool rotate = false;
    bool blend = false;
    bool doubleSize = false;
};

typedef SpriteAttributes OBJ_ATTR;

#define INLINE static inline

typedef int FIXED;
#define FIXED_BITS	8
#define float2fx(f)	(FIXED)(f*((float)(1 << FIXED_BITS)))
#define int2fx(i)		((int)(i) << FIXED_BITS)
#define fx2int(f)		((int)(f) >> FIXED_BITS)
#define fx2float(f)	(float)(f)/(1 << FIXED_BITS)

INLINE FIXED fxmul(FIXED fa, FIXED fb)
{	return (fa*fb)>>FIXED_BITS;				}

INLINE FIXED fxdiv(FIXED fa, FIXED fb)
{	return ((fa)*(1<<FIXED_BITS))/(fb);			}

#define ALIGN(n)	__attribute__((aligned(n)))

static inline void log(int n){
    std::cout << std::to_string(n) << "\n";
}

static inline void log(std::string str){
    std::cout << str << "\n";
}

extern TLN_Palette palettes[32];

extern TLN_Tileset tilesets[4];

extern TLN_Tilemap tilemaps[32];

extern TLN_SpriteData spriteData[128];

extern TLN_Spriteset spriteset;

extern TLN_Bitmap spriteBitmap;

extern void key_poll();

extern u32 Sqrt(u32 n);

extern int rowStart;
extern int rowEnd;

// extern bool canDraw;

INLINE void setTile(int sbb, int x, int y, u32 value){

    TLN_Tile t = new Tile();
    t->value = value;
    // t->index = 1;
    // t->flags = 0;
    //

    //TODO:
    if(y < 0)
        y += 64;

    TLN_SetTilemapTile(tilemaps[sbb], y, x, t);

    delete t;
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

INLINE u32 tileBuild(u16 value){
    TLN_Tile t = new Tile();
    t->index = value & 0xff;
    t->palette = (value >> 12) & 0xf;
    t->flipx = ((value >> 8) & 0xf) & 0x4;
    t->flipy = ((value >> 8) & 0xf) & 0x8;

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

INLINE void memset16(void * dst, u16 n, int count){
    u16* dest = (u16*)dst;
    while(count--){
        *dest++ = n;
    }
}

INLINE void memset32(void * dst, u32 n, int count){
    u32* dest = (u32*)dst;
    while(count--){
        *dest++ = n;
    }
}

INLINE int qran(){
    return rand();
}

INLINE void sqran(int seed){
    srand(seed);
}

extern std::vector<std::string> keyToString;

extern std::list<sf::Keyboard::Key> currentKeys;
extern std::list<sf::Keyboard::Key> previousKeys;

void key_poll();

//! Gives the keys of \a key that are currently down
INLINE u32 key_is_down(u32 key){
    if(key == KEY_FULL)
        return (!currentKeys.empty());

    auto it = currentKeys.begin();

    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));

    return (it != currentKeys.end());
}

//! Gives the keys of \a key that are currently up
INLINE u32 key_is_up(u32 key){
    if(key == KEY_FULL)
        return (currentKeys.empty());

    auto it = currentKeys.begin();

    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));

    return (it == currentKeys.end());
}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_down(u32 key){
    if(key == KEY_FULL)
        return (!previousKeys.empty());

    auto it = previousKeys.begin();

    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));

    return (it != previousKeys.end());
}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_up(u32 key){
    if(key == KEY_FULL)
        return (previousKeys.empty());

    auto it = previousKeys.begin();

    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));

    return (it == previousKeys.end());
}

//! Gives the keys of \a key that are different from before
INLINE u32 key_transit(u32 key){
    if(key == KEY_FULL)
        return (!previousKeys.empty() ^ !currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    bool curr = (it != currentKeys.end());

    return (prev ^ curr);
}

//! Gives the keys of \a key that are being held down
INLINE u32 key_held(u32 key){
    if(key == KEY_FULL)
        return (!previousKeys.empty() && !currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    bool curr = (it != currentKeys.end());

    return (prev && curr);
}

//! Gives the keys of \a key that are pressed (down now but not before)
INLINE u32 key_hit(u32 key){
    if(key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    bool curr = (it != currentKeys.end());

    return (!prev && curr);
}

//! Gives the keys of \a key that are being released
INLINE u32 key_released(u32 key){
    if(key == KEY_FULL)
        return (!previousKeys.empty() && currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    bool curr = (it != currentKeys.end());

    return (prev && !curr);
}

INLINE COLOR RGB15(int red, int green, int blue){
    return red + (green<<5) + (blue<<10);
}

INLINE COLOR RGB15_SAFE(int red, int green, int blue){
    return (red&31) + ((green&31)<<5) + ((blue&31)<<10);
}


INLINE int min(int a, int b){
    return (a < b)? a : b;
}

INLINE int max(int a, int b){
    return (a > b)? a : b;
}

INLINE void setSpriteTile(int index, u8 tx, u8 ty, u8 width, const void *src){
    u8* source = (u8*) src;
    uint8_t tmp[8 * 8];
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 4; j++){
            tmp[i * 8 + j * 2] = source[i * 4 + j] & 0xf;
            tmp[i * 8 + j * 2 + 1] = (source[i * 4 + j] >> 4) & 0xf;
        }
    }

    // TLN_SpriteData temp;

    // memcpy(temp.name,spriteData->name,64);

    // temp.w = spriteData->w;
    // temp.h = spriteData->h;
    // temp.x = spriteData->x;
    // temp.y = spriteData->h;

    TLN_SpriteData d;
    memcpy(d.name,"tile",4);
    d.w = 8;
    d.h = 8;
    // d.w = lengthX * 8;
    // d.h = lengthY * 8;
    // d.x = index * 8;
    // d.y = 0;

    d.x = 8 * (tx + index);
    d.y = 8 * ty;

    TLN_SetSpritesetData(spriteset, 0, &d, tmp, 8);

    // memcpy(spriteData->name,temp.name,64);

    // spriteData->w = temp.w;
    // spriteData->h = temp.h;
    // spriteData->x = temp.x;
    // spriteData->h = temp.x;
    // memcpy32(&tile_mem[4][index + ty * 4 + tx], src, 8);
}

INLINE void setSpritePixel(int index, u8 tx, u8 ty, u8 width, u8 x, u8 y, u8 color){
    u8 *dst = TLN_GetBitmapPtr(spriteBitmap, (tx + index) * 8 + x, ty * 8 + y);

    *dst = color;
}

INLINE void setTilesetPixel(int cbb, int index, int x, int y, u8 color){
    u8 *dst = TLN_GetTilesetPtr(tilesets[cbb],index,x,y);

    *dst = color;
}

INLINE void setTileRow(int tileset, int index, int row, u32 data){
    u8 *dst = TLN_GetTilesetPtr(tilesets[tileset],index,0,row);

    for(int i = 0; i < 8; i++)
        *dst++ = (data >> (4 * i)) & 0xf;
}

INLINE u32 bf_clamp(int x, uint len){
	u32 y=x>>len;
	if(y)
		x= (~y)>>(32-len);
	return x;
}

INLINE int clamp(int x, int min, int max)
{	return (x>=max) ? (max-1) : ( (x<min) ? min : x );	}

extern void color_fade_fast(int palette, int index, const COLOR * src, COLOR color, int count, u32 alpha);

extern void clr_rgbscale(COLOR *dst, const COLOR *src, uint nclrs, COLOR clr);

extern void rumble_set_state(int);

// extern void sfx(int n);
// extern void sfx
