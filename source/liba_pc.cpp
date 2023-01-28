#include "platform.hpp"

#ifdef PC
#include "Tilengine.h"
#include "stddef.h"
#include "def.h"
#include "logging.h"
#include <cstring>
#include <cmath>
#include "stdarg.h"
#include <cstdio>
#include <fstream>
#include <list>

#include <SDL2/SDL.h>
// #include "main.hpp"
//

static bool blendSprites = false;

int blendInfo = 0;

void onHBlank(int line);

class ColorSplit{

public:
    int r = 0;
    int g = 0;
    int b = 0;

    ColorSplit(int _r, int _g, int _b){
        r = _r;
        g = _g;
        b = _b;

    }

    ColorSplit(u16 c){
        r = (((c) & 0x1f) << 3);
        g = (((c >> 5) & 0x1f) << (3));
        b = (((c >> 10) & 0x1f) << (3));
    }
};

static inline int rgb5to8(u16 color){
    return (((color) & 0x1f) << 3) + (((color >> 5) & 0x1f) << (3+8)) + (((color >> 10) & 0x1f) << (3+16));
}

class LayerInfo{
public:
    int id = -1;
    bool enabled = true;
    int priority = -1;
    int sbb = 0;
    int cbb = 0;
};

TLN_Palette palettes[32];

TLN_Tileset tilesets[4];

TLN_Tilemap tilemaps[32];

TLN_Spriteset spriteset;

TLN_SpriteData spriteData[128];

TLN_Bitmap spriteBitmap;

LayerInfo layerInfo[4];

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

void platformInit(){
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

    spriteBitmap = TLN_CreateBitmap(1024 * 8, 64, 8);

    TLN_SetBitmapPalette(spriteBitmap, palettes[16]);//  NECESSARY ! ! !
    spriteset = TLN_CreateSpriteset(spriteBitmap, spriteData, 128);

    setInputs();

    TLN_CreateWindow(NULL, 0);

    TLN_ConfigCRTEffect(TLN_CRT_APERTURE,false);
    TLN_DisableCRTEffect();

    TLN_SetRasterCallback(onHBlank);

    TLN_EnableInput(PLAYER1, true);
}

void deinitialize(){
    TLN_DeleteWindow();
    TLN_Deinit();

    // SDL_CloseAudioDevice(deviceId);
    // SDL_FreeWAV(wavBuffer);
    // SDL_Quit();
}

bool closed(){
    // frame_start = SDL_GetPerformanceCounter();
    bool result = TLN_ProcessWindow();

    if(!result){
        deinitialize();
    }

    return result;
}

void toggleBG(int layer, bool state){
    int n = layerInfo[layer].id;

    if(state){
        TLN_EnableLayer(n);
    }else{
        TLN_DisableLayer(n);
    }

    layerInfo[layer].enabled = state;
}

void toggleSprites(bool state){

}

INLINE u8 buildLayerPrio(int layer, int priority){
    return ((priority & 0x3) << 4) + 4 - (layer & 0x3);
}

void buildBG(int layer, int cbb, int sbb, int size, int prio, int mos){
    int p = buildLayerPrio(layer, prio);
    layerInfo[layer].priority = p;
    layerInfo[layer].sbb = sbb;
    layerInfo[layer].cbb = cbb;

    int a[4] = {-1,-1,-1,-1};

    for(int l = 0; l < 4; l++){
        for(int pos = 0; pos < 4; pos ++){
            if(a[pos] == -1){
                a[pos] = l;
                break;
            }else if(layerInfo[a[pos]].priority > layerInfo[l].priority){
                for(int rpos = 3; rpos > pos; rpos--){
                    a[rpos] = a[rpos-1];
                }
                a[pos] = l;
                break;
            }
        }
    }

    for(int pos = 0; pos < 4; pos++){
        if(a[pos] == -1)
            break;

        // log("setting layer " + std::to_string(a[pos]) + " to slot " + std::to_string(pos));
        layerInfo[a[pos]].id = pos;
        TLN_SetTilemapTileset(tilemaps[layerInfo[a[pos]].sbb], tilesets[layerInfo[a[pos]].cbb]);
        TLN_SetLayerPriority(pos, (a[pos] == 2));
        TLN_SetLayerTilemap(pos, tilemaps[layerInfo[a[pos]].sbb]);

        toggleBG(pos,layerInfo[pos].enabled);
    }
}

void vsync(){
    onVBlank();

    TLN_DrawFrame(0);


	// Uint64 end = SDL_GetPerformanceCounter();

	// float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

	// // Cap to 60 FPS
	// SDL_Delay(floor(16.666f - elapsedMS));
}

void showSprites(int n ){
    // oam_init(oam_mem,128);
    // OBJ_ATTR* dest = (OBJ_ATTR*) MEM_OAM;

    std::list<int> spriteList;

    for(int i = 3; i >= 0; i--){
        // log(std::to_string(i));
        for(int counter = 127; counter >= 0; counter--){
            OBJ_ATTR *sprite = &obj_buffer[counter];
            if(sprite->priority == i){
                spriteList.push_back(sprite->id);
                // log(std::to_string(sprite->id) + " " + std::to_string(sprite->priority));
            }
        }
    }

    int counter = 0;
    for(auto const & id : spriteList){
    // for(int counter = 0; counter < n; counter++){
        OBJ_ATTR *sprite = &obj_buffer[id];

        if(!sprite->enabled){
            TLN_DisableSprite(counter);
            counter++;
            continue;
        }

        // TLN_SpriteData d;
        spriteData[counter].x = (sprite->tile_start) * 8;
        spriteData[counter].y = 0;
        spriteData[counter].w = sprite->sx;
        spriteData[counter].h = sprite->sy;

        TLN_SetSpriteSet(counter, spriteset);
        TLN_SetSpritesetData(spriteset, counter, &spriteData[counter], 0,0);
        TLN_SetSpritePicture(counter, counter);
        TLN_SetSpritePalette(counter, palettes[sprite->palette + 16]);
        if(sprite->affine && !sprite->rotate){
            TLN_SetSpritePivot(counter, 0.5, 0.5);
            TLN_SetSpriteScaling(counter, sprite->scalex, sprite->scaley);
            TLN_SetSpritePosition(counter, sprite->x + sprite->sx/2, sprite->y + sprite->sy/2);
        }else{
            TLN_SetSpritePosition(counter, sprite->x, sprite->y);
            TLN_SetSpritePivot(counter, 0, 0);
            TLN_SetSpriteScaling(counter, 1, 1);
        }

        TLN_SetSpriteBlendMode(counter, (blendSprites || sprite->blend)?BLEND_MIX25:BLEND_NONE , 0);
        TLN_SetSpriteFlags(counter, (sprite->flipx * FLAG_FLIPX) + (sprite->flipy * FLAG_FLIPY) + (sprite->rotate * FLAG_ROTATE) + ((sprite->priority == 0) * FLAG_PRIORITY));
        counter++;
    }
}

// void loadTiles(uint8_t * tiles, int size){
// }

void loadPalette(int palette, int index, const void * src, int count){
    for(int i = 0; i < (int) 16; i++){
        u16 r = (u16) ((u16 *) src)[i];
        ColorSplit c = ColorSplit(r);
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
    if(sprite->enabled){
        sprite->enabled = false;
        // TLN_DisableSprite(sprite->id);
    }
};

void sprite_unhide(OBJ_ATTR * sprite, int mode){
    sprite->enabled = true;
    // TLN_SetSpriteSet(sprite->id, spriteset);
};

void sprite_set_pos(OBJ_ATTR * sprite, int x, int y){
    sprite->x = x;
    sprite->y = y;
};

void sprite_set_attr(OBJ_ATTR* sprite, int shape, int size, int tile_start, int palette, int priority){
    sprite->sx = sizeTable[shape][size][0];
    sprite->sy = sizeTable[shape][size][1];
    sprite->tile_start = tile_start;
    sprite->palette = palette;
    sprite->priority = priority;
    sprite->affine = false;
    sprite->flipx = false;
    sprite->flipy = false;
    sprite->rotate = false;
    sprite->scalex = 1;
    sprite->scaley = 1;
    sprite->blend = false;
};

void setLayerScroll(int layer, int x, int y){
    TLN_SetLayerPosition(layerInfo[layer].id, x, y);
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

    TLN_SpriteData d;
    memcpy(d.name,"tile",4);
    d.w = 8;
    d.h = 8;

    int counter = 0;
    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            d.x = 8 * (j + index);
            d.y = 8 * i;

            TLN_SetSpritesetData(spriteset, 127, &d, &tmp[counter * 64], 8);
            counter++;
        }
    }
}

void loadSpriteTilesPartial(int index, const void *src, int tx, int ty, int lengthX, int lengthY){
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

    TLN_SpriteData d;
    memcpy(d.name,"tile",4);
    d.w = 8;
    d.h = 8;

    int counter = 0;
    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            d.x = 8 * (j + index + tx);
            d.y = 8 * (i + ty);

            TLN_SetSpritesetData(spriteset, 127, &d, &tmp[counter * 64], 8);
            counter++;
        }
    }
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

void enableBlend(int info){

    int top = info & 0x3f;
    int bot = (info >> 8) & 0x3f;
    int mode = (info >> 6) & 0x3;

    for(int i = 0; i < 4; i++){
        if(((top >> i) & 0b1)){
            TLN_SetLayerBlendMode(layerInfo[i].id,BLEND_MIX75,0);
        }else if(((bot >> i) & 0b1))
            TLN_SetLayerBlendMode(layerInfo[i].id,BLEND_MIX,0);
        else{
            // log("disabling blend for layer: " + std::to_string(i));
            TLN_SetLayerBlendMode(layerInfo[i].id,BLEND_NONE,0);
        }
    }

    blendSprites = (bot >> 4) & 0b1;

    blendInfo = info;
}

void setMusicTempo(int tempo){

}

void setMusicVolume(int volume){

}

void setSFXVolume(int volume){

}

void startSong(int song, bool loop){

}

u32 Sqrt(u32 n){
    return std::sqrt(n);
}

void sfx(int n){
    // Uint32 wavLength;
    // Uint8 *wavBuffer;
    // SDL_LoadWAV("audio/levelUP.wav", &wavSpec, &wavBuffer, &wavLength);
}

void sfxRate(int sound, float rate){

}

void setTiles(int tilemap, int index, int count, u32 value){
    TLN_Tile t = new Tile();
    t->value = value;
    // t->index = 1;
    // t->flags = 0;

    for(int i = index; i < count+index; i++){
        TLN_SetTilemapTile(tilemaps[tilemap], (i / 32), (i % 32), t);
    }
}

void clearSprites(int n){
    for(int counter = 0; counter < n; counter++){
        sprite_hide(&obj_buffer[counter]);
    }
}

void setPaletteColor(int palette, int index, int color, int count){

}

void clearSpriteTiles(int index, int lengthX, int lengthY){
    // u8* source = (u8*) src;
    const int count = lengthX * lengthY;
    uint8_t tmp[8 * 8];
    memset(tmp,0,8 * 8);

    // for(int ii = 0; ii < count; ii++){
    //     for(int i = 0; i < 8; i++){
    //         for(int j = 0; j < 4; j++){
    //             tmp[ii * 64 + i * 8 + j * 2] = source[ii * 32 + i * 4 + j] & 0xf;
    //             tmp[ii * 64 + i * 8 + j * 2 + 1] = (source[ii * 32 + i * 4 + j] >> 4) & 0xf;

    //         }
    //     }
    // }

    TLN_SpriteData d;
    memcpy(d.name,"clear",4);
    d.w = 8;
    d.h = 8;

    int counter = 0;
    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            d.x = 8 * (j + index);
            d.y = 8 * i;

            TLN_SetSpritesetData(spriteset, 127, &d, &tmp, 8);
            counter++;
        }
    }

    // TLN_SetSpritesetData(spriteset, 0, &d, &tmp, 8);

}

void sprite_enable_mosaic(OBJ_ATTR *sprite){

}

void sprite_set_size(OBJ_ATTR *sprite, FIXED size, int aff_id){
    if(size == 0)
        return;

    float scale = 1/(fx2float(size));

    sprite->scalex = scale;
    sprite->scaley = scale;
}

void sprite_rotscale(OBJ_ATTR *sprite, FIXED sizex, FIXED sizey, int angle, int aff_id){
    sprite->affine = false;
    sprite->flipx = (sizex < 0);
    // sprite->flipy = (sizey < 0);

    int n = angle / 0x4000;

    if((abs(n)) % 2 == 1){
        sprite->rotate = true;
    }

    // if(n > 1){
    //     sprite->flipy = !sprite->flipy;
    // }

    // sprite_set_size(sprite, sizex, aff_id);
};

void sprite_enable_affine(OBJ_ATTR *sprite, int affineId, bool doubleSize){
    sprite->affine = true;
}


extern "C" {

void posprintf(char * buff, const char * str, ...){

    va_list args;
    va_start(args,str);
    vsnprintf(buff, 64, str, args);
    va_end(args);
    // snprintf(buff, 64 , );
}

}

void stopSong(){};
void pauseSong(){};
void resumeSong(){};

void color_fade_palette(int palette, int index, const COLOR * src, COLOR color, int count, u32 alpha){
    u16 temp[count];

    color_fade(temp,src,color,count,alpha);

    loadPalette(palette, index, temp, count);
};

void color_fade(COLOR * dst, const COLOR * src, COLOR color, int count, u32 alpha){
	if(!count)
		return;

	u32 clra, parta, part;
	const u32 rbmask= (RED_MASK|BLUE_MASK), gmask= GREEN_MASK;
	const u32 rbhalf= 0x4010, ghalf= 0x0200;

	// precalc color B parts
	u32 partb_rb= color & rbmask, partb_g=  color & gmask;

	for(int ii = 0; ii < count; ii++)
	{
		clra= src[ii];

		// Red and blue
		parta= clra & rbmask;
		part = (partb_rb-parta)*alpha + parta*32 + rbhalf;
		 color  = (part/32) & rbmask;

		// Green
		parta= clra & gmask;
		part = (partb_g-parta)*alpha + parta*32 + ghalf;
		 color |= (part/32) & gmask;

		dst[ii]= color;
	}
};

void color_adj_brightness(int palette, int index, const COLOR *src, int count, u32 alpha){};
void clearSpriteTile(int index, int tx, int ty, int width){
    uint8_t tmp[8 * 8];
    memset(tmp,0,64);

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

    TLN_SetSpritesetData(spriteset, 127, &d, tmp, 8);

    // memcpy(spriteData->name,temp.name,64);

    // spriteData->w = temp.w;
    // spriteData->h = temp.h;
    // spriteData->x = temp.x;
    // spriteData->h = temp.x;
    // memcpy32(&tile_mem[4][index + ty * 4 + tx], src, 8);

};

void clearTiles(int tileset, int index, int count){
    u8 tmp[8*8];

    memset(tmp,0,64);
    for(int ii = 0; ii < count; ii++){
        // for(int i = 0; i < 8; i++){
        //     for(int j = 0; j < 4; j++){
        //         tmp[i * 8 + j * 2] = source[ii * 32 + i * 4 + j] & 0xf;
        //         tmp[i * 8 + j * 2 + 1] = (source[ii * 32 + i * 4 + j] >> 4) & 0xf;
        //     }
        // }

        TLN_SetTilesetPixels(tilesets[tileset], index+ii, tmp, 8);
    }

};

void clearTilemapEntries(int tilemap, int index, int count){
    for(int i = index; i < count+index; i++){
        setTile(tilemap, i/32, i%32, 0);
    }
}

void stopDMA(){};
void setMosaic(int sx, int sy){};

void sleep(){};
void rumble_set_state(int n){};

static uint64_t frame_start = 0;
static uint64_t frame_end = 0;

void onVBlank(){
    if (canDraw) {
        canDraw = 0;

        scene->draw();
    }

    frameCounter++;

    frame_end = SDL_GetPerformanceCounter();

    float elapsedMS = (frame_end - frame_start) / (float) SDL_GetPerformanceFrequency() * 1000.0f;

    int delay = (std::floor(16.666f - elapsedMS));

    if(frame_start > 0 && delay > 0)
        SDL_Delay(delay);

    frame_start = SDL_GetPerformanceCounter();
};

void onHBlank(int line){
    if(!gradientEnabled){
        // TLN_DisableBGColor();
        TLN_SetBGColor(0,0,0);
        return;
    }

    ColorSplit c = ColorSplit(gradientTable[line]);
    TLN_SetBGColor(c.r, c.g, c.b);
}

void loadSavefile(){
    // memset32(savefile, 0 , sizeof(Save)/4);

    std::ifstream input("Apotris.sav",std::ios::binary | std::ios::in);

    char * src = (char * ) savefile;

    input.read(src,sizeof(Save));

    if(!input){
        log("Error when trying to load save.");
        return;
    }

    input.close();
}

void saveSavefile(){
    addStats();

    std::ofstream output("Apotris.sav",std::ios::binary | std::ios::out);

    char * dst = (char * ) savefile;

    output.write(dst,sizeof(Save));

    if(!output){
        log("Error when trying to write save.");
        return;
    }

    output.close();
}

void toggleHBlank(bool state){
    if(state)
        gradientEnabled = true;
}

s16 sinLut(int angle){
    return ((int)((float)(1 << 12) * (sin((float)angle / 512)))) & 0xffff;
}

#endif
