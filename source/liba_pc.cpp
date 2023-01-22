#include "platform.hpp"

#ifdef PC
#include "Tilengine.h"
#include "stddef.h"
#include <cstring>

#include <SDL2/SDL.h>
#include "main.hpp"

class COLOR{

public:
    int r = 0;
    int g = 0;
    int b = 0;

    COLOR(int _r, int _g, int _b){
        r = _r;
        g = _g;
        b = _b;

    }

    COLOR(u16 c){
        r = (((c) & 0x1f) << 3);
        g = (((c >> 5) & 0x1f) << (3));
        b = (((c >> 10) & 0x1f) << (3));
    }
};

static inline int rgb5to8(u16 color){
    return (((color) & 0x1f) << 3) + (((color >> 5) & 0x1f) << (3+8)) + (((color >> 10) & 0x1f) << (3+16));
}

TLN_Palette palettes[32];

TLN_Tileset tilesets[4];

TLN_Tilemap tilemaps[32];

TLN_Spriteset spriteset;

TLN_SpriteData spriteData[128];

u16 __key_curr = 0;
u16 __key_prev = 0;

// static Uint64 frame_start;

void setInputs(){
    TLN_DefineInputKey(PLAYER1, INPUT_UP, SDLK_c);
    TLN_DefineInputKey(PLAYER1, INPUT_DOWN, SDLK_k);
    TLN_DefineInputKey(PLAYER1, INPUT_LEFT, SDLK_j);
    TLN_DefineInputKey(PLAYER1, INPUT_RIGHT, SDLK_l);
    TLN_DefineInputKey(PLAYER1, INPUT_BUTTON1, SDLK_i);
    TLN_DefineInputKey(PLAYER1, INPUT_BUTTON2, SDLK_SEMICOLON);
    TLN_DefineInputKey(PLAYER1, INPUT_BUTTON3, SDLK_z);
    TLN_DefineInputKey(PLAYER1, INPUT_BUTTON4, SDLK_s);
    TLN_DefineInputKey(PLAYER1, INPUT_START, SDLK_RETURN);
    TLN_DefineInputKey(PLAYER1, INPUT_QUIT, SDLK_ESCAPE);
    TLN_DefineInputKey(PLAYER1, INPUT_CRT, SDLK_BACKSPACE);
}

void initialize(){
    TLN_Init(240,160,4,128,0);

    for(int i = 0; i < 32; i++){
        palettes[i] = TLN_CreatePalette(16);
        if(i < 16)
            TLN_SetGlobalPalette(i, palettes[i]);
    }

    for(int i = 0; i < 4; i++){
        tilesets[i] = TLN_CreateTileset(512, 8, 8, palettes[0], NULL, NULL);
    }

    Tile tiles[32*32];
    for(int i = 0; i < 32; i++)
        for(int j = 0; j < 32; j++)
            tiles[i * 32 + j].value = 0;

    for(int i = 0; i < 32; i++){
        tilemaps[i] = TLN_CreateTilemap(32, 32, tiles, 0, tilesets[0]);
    }

    TLN_Bitmap bm = TLN_CreateBitmap(1024 * 8, 64, 8);

    TLN_SetBitmapPalette(bm, palettes[16]);//  NECESSARY ! ! !
    spriteset = TLN_CreateSpriteset(bm, spriteData, 128);

    setInputs();

    TLN_CreateWindow(NULL, 0);

    TLN_ConfigCRTEffect(TLN_CRT_APERTURE,false);
    TLN_DisableCRTEffect();

    TLN_EnableInput(PLAYER1, true);
}

void deinitialize(){
    TLN_DeleteWindow();
    TLN_Deinit();
}

bool closed(){
    // frame_start = SDL_GetPerformanceCounter();
    return TLN_ProcessWindow();
}

void toggleBG(int layer, bool state){
    if(state){
        TLN_EnableLayer(layer);
    }else{
        TLN_DisableLayer(layer);
    }
}

void toggleSprites(bool state){

}

void buildBG(int layer, int cpp, int spp, int size, int prio, int mos){
    TLN_SetTilemapTileset(tilemaps[spp], tilesets[cpp]);
    TLN_SetLayerPriority(layer, prio);
    TLN_SetLayerTilemap(layer, tilemaps[spp]);
}

void vsync(){
    onVBlank();

    TLN_DrawFrame(0);


	// Uint64 end = SDL_GetPerformanceCounter();

	// float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

	// // Cap to 60 FPS
	// SDL_Delay(floor(16.666f - elapsedMS));
}

void showSprites(){
    // oam_init(oam_mem,128);
    // OBJ_ATTR* dest = (OBJ_ATTR*) MEM_OAM;
    int counter = 0;
    for(auto const& sprite : sprite_list){
        // TLN_SpriteData d;
        spriteData[counter].x = (sprite.sprite->tile_start) * 8;
        spriteData[counter].y = 0;
        spriteData[counter].w = sprite.sprite->sx;
        spriteData[counter].h = sprite.sprite->sy;

        TLN_SetSpriteSet(counter, spriteset);
        TLN_SetSpritesetData(spriteset, counter, &spriteData[counter], 0,0);
        TLN_SetSpritePicture(counter, counter);
        TLN_SetSpritePalette(counter, palettes[sprite.sprite->palette + 16]);
        TLN_SetSpritePosition(counter, sprite.sprite->x,sprite.sprite->y);
        TLN_SetSpriteBlendMode(counter, BLEND_NONE, 0);
        TLN_SetSpriteFlags(counter, FLAG_PRIORITY);
        counter++;
    }
}

// void loadTiles(uint8_t * tiles, int size){
// }

void loadPalette(int palette, int index, const void * src, int count){
    for(int i = 0; i < (int) 16; i++){
        u16 r = (u16) ((u16 *) src)[i];
        COLOR c = COLOR(r);
        TLN_SetPaletteColor(palettes[palette], i, c.r, c.g, c.b);
    }
}

void loadTiles(int tileset, int index, const void * src, int count){
    u8 tmp[8*8];
    u8 *source = (u8*)src;
    for(int ii = 0; ii < count; ii++){
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 4; j++){
                tmp[i * 8 + j * 2] = source[ii * 32 + i * 4 + j] & 0xf;
                tmp[i * 8 + j * 2 + 1] = (source[ii * 32 + i * 4 + j] >> 4) & 0xf;
            }
        }

        TLN_SetTilesetPixels(tilesets[tileset], index+ii, tmp, 8);
    }
}

void loadTiles8(int tileset, int index, const void * src, int count){
    u8 *source = (u8*)src;
    for(int i = 0; i < count; i++){
        TLN_SetTilesetPixels(tilesets[tileset], index+i,(u8*) &source[64 * i], 8);
    }
}

void clearTilemap(int tilemap){
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 32; j++){
            setTile(tilemap, i, j, 0);
        }
    }
}

void sprite_hide(OBJ_ATTR * sprite){
    TLN_DisableSprite(sprite->id);
};

void sprite_unhide(OBJ_ATTR * sprite, int mode){
    TLN_SetSpriteSet(sprite->id, spriteset);
};

void sprite_set_pos(OBJ_ATTR * sprite, int x, int y){
    sprite->x = x;
    sprite->y = y;
};

void sprite_set_attr(OBJ_ATTR* sprite, int shape, int size, int tile_start, int palette, int priority){
    // sprite->id;
    sprite->sx = sizeTable[shape][size][0];
    sprite->sy = sizeTable[shape][size][1];
    sprite->tile_start = tile_start;
    sprite->palette = palette;
    sprite->priority = priority;
};

void setLayerScroll(int layer, int x, int y){
    TLN_SetLayerPosition(layer, x, y);
}

void loadSpriteTiles(int index, const void *src, int lengthX, int lengthY){
    u8* source = (u8*) src;
    const int count = lengthX * lengthY;
    uint8_t tmp[8 * 8 * count];
    for(int ii = 0; ii < count; ii++){
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 4; j++){
                tmp[ii * 64 + i * 8 + j * 2] = source[ii * 32 + i * 4 + j] & 0xf;
                tmp[ii * 64 + i * 8 + j * 2 + 1] = (source[ii * 32 + i * 4 + j] >> 4) & 0xf;

            }
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

    int counter = 0;
    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            d.x = 8 * (j + index);
            d.y = 8 * i;

            TLN_SetSpritesetData(spriteset, 0, &d, &tmp[counter * 64], 8);
            counter++;
        }
    }

    // memcpy(spriteData->name,temp.name,64);

    // spriteData->w = temp.w;
    // spriteData->h = temp.h;
    // spriteData->x = temp.x;
    // spriteData->h = temp.x;
}

void key_poll(){
    __key_prev = __key_curr;

    __key_curr = 0;

    __key_curr |= (TLN_GetInput(INPUT_BUTTON1)) * KEY_A;
    __key_curr |= (TLN_GetInput(INPUT_BUTTON2)) * KEY_B;
    __key_curr |= (TLN_GetInput(INPUT_BUTTON3)) * KEY_R;
    __key_curr |= (TLN_GetInput(INPUT_BUTTON4)) * KEY_L;
    __key_curr |= (TLN_GetInput(INPUT_START)) * KEY_START;
    __key_curr |= (TLN_GetInput(INPUT_UP)) * KEY_UP;
    __key_curr |= (TLN_GetInput(INPUT_DOWN)) * KEY_DOWN;
    __key_curr |= (TLN_GetInput(INPUT_LEFT)) * KEY_LEFT;
    __key_curr |= (TLN_GetInput(INPUT_RIGHT)) * KEY_RIGHT;
}

void enableLayerBlend(int layer){
    TLN_SetLayerBlendMode(layer,BLEND_ADD,0);
}

//TODO:
void color_fade(int palette, int index, const COLOR *src, COLOR color, int count, u32 alpha){

}

void setMusicTempo(int tempo){

}

void setMusicVolume(int volume){

}

void setSFXVolume(int volume){

}

void startSong(int song, bool loop){

}

#endif
