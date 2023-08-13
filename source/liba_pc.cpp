#include "platform.hpp"
#include <string>

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
#include "audio_files.h"
#include "scene.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "sfMod.h"
#include <map>

#define TLN_EXCLUDE_WINDOW

std::list<sf::Sound*> soundList;
sf::SoundBuffer sfxBuffers[SFX_COUNT];
sfmod::Mod musicArray[MUSIC_COUNT];

sf::Shader shader;

// Mix_Chunk *soundEffectChunks[SFX_COUNT];
// Mix_Music *musicChunks[MUSIC_COUNT];

static int offsetx = (SCREEN_WIDTH - 240)/2;
static int offsety = (SCREEN_HEIGHT - 160)/2;

int spriteVOffset = 0;

bool render = true;

std::list<sf::Keyboard::Key> currentKeys;
std::list<sf::Keyboard::Key> previousKeys;

void resize(int width, int height);

void loadAudio(){

    for(int i = 0; i < SFX_COUNT; i++){
        if(!sfxBuffers[i].loadFromFile(SoundEffectPaths[i])){
            std::string str = "Failed to load ";
            str += SoundEffectPaths[i];
            log(str);
        }
    }

    for(int i = 0; i < MUSIC_COUNT; i++){
        if(!musicArray[i].loadFromFile(SongPaths[i])){
            std::string str = "Failed to load ";
            str += SongPaths[i];
            log(str);
        }
    }
}

void freeAudio(){
    // for(int i = 0; i < SFX_COUNT; i++){
    //     Mix_FreeChunk(soundEffectChunks[i]);
    // }

    // for(int i = 0; i < MUSIC_COUNT; i++){
    //     Mix_FreeMusic(musicChunks[i]);
    // }
}

static bool blendSprites = false;

int blendInfo = 0;
int currentSong = -1;
int sfxVolume = 1024;
int musicVolume = 0;

const int sfxVolumeMultiplier = 4;

void onHBlank(int line);
uint8_t customBlend(uint8_t a,uint8_t b);

class ColorSplit{

public:
    int r = 0;
    int g = 0;
    int b = 0;

    //Red and Blue are swapped intentionally to fix Tilengine -> SFML issue
    ColorSplit(int _r, int _g, int _b){
        b = _r;
        g = _g;
        r = _b;

    }

    ColorSplit(u16 c){
        b = (((c) & 0x1f) << 3);
        g = (((c >> 5) & 0x1f) << (3));
        r = (((c >> 10) & 0x1f) << (3));
    }

    ColorSplit(u32 *c){
        b = (*c >> (16 + 3)) & 0x1f;
        g = (*c >> ( 8 + 3)) & 0x1f;
        r = (*c >> ( 0 + 3)) & 0x1f;
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

int windowScale = 3;
// int windowWidth = 256;
// int windowHeight = 192;
int windowWidth = 214;
int windowHeight = 160;

int rowStart = (512 - windowHeight)/2;
int rowEnd = (512 + windowHeight)/2;

u8 * framebuffer;
sf::RenderWindow window(sf::VideoMode(windowWidth * windowScale, windowHeight * windowScale), "Apotris PC");
sf::Texture frameTexture;

bool isFullscreen = false;

#define TILEMAP_SIZE 64

void platformInit(){

    TLN_Init(SCREEN_WIDTH,SCREEN_HEIGHT,4,128,0);

    framebuffer = (sf::Uint8*) malloc(4 * SCREEN_WIDTH * SCREEN_HEIGHT);

    TLN_SetRenderTarget(framebuffer, 4 * SCREEN_WIDTH);

    window.setFramerateLimit(60);

    frameTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT);

    // window.setActive(false);

    for(int i = 0; i < 32; i++){
        palettes[i] = TLN_CreatePalette(16);
        if(i < 16)
            TLN_SetGlobalPalette(i, palettes[i]);
    }

    for(int i = 0; i < 4; i++){
        tilesets[i] = TLN_CreateTileset(512, 8, 8, palettes[0], NULL, NULL);
    }

    Tile tiles[TILEMAP_SIZE * TILEMAP_SIZE];
    for(int i = 0; i < TILEMAP_SIZE; i++)
        for(int j = 0; j < TILEMAP_SIZE; j++)
            tiles[i * TILEMAP_SIZE + j].value = 0;

    for(int i = 0; i < 32; i++){
        tilemaps[i] = TLN_CreateTilemap(64, 64, tiles, 0, tilesets[0]);
    }

    spriteBitmap = TLN_CreateBitmap(1024 * 8, 64, 8);

    TLN_SetBitmapPalette(spriteBitmap, palettes[16]);//  NECESSARY ! ! !
    spriteset = TLN_CreateSpriteset(spriteBitmap, spriteData, 128);

    TLN_SetRasterCallback(onHBlank);

    sf::Image icon;
    icon.loadFromFile("assets/favicon32.bmp");

    window.setIcon(32, 32, icon.getPixelsPtr());

    loadAudio();

    TLN_SetCustomBlendFunction(customBlend);

    if(!sf::Shader::isAvailable()){
        log("Shaders not available on system.");
    }else if(ENABLE_SHADERS){
        if(!shader.loadFromFile("lib/bloom.glsl", sf::Shader::Fragment)){
            log("Couldn't load Bloom shader.");
        }else{
            log("Loaded Bloom Shader.");
            shader.setUniform("pixelWidth",1.f / window.getSize().x);
            shader.setUniform("pixelHeight",1.f / window.getSize().y);
        }

    }

    if(sf::Joystick::isConnected(0)){
        log("controller connected");

        int buttonCount = sf::Joystick::getButtonCount(0);

        log("button count: " + std::to_string(buttonCount));

        int axii = 0;

        for(int i = 0; i < 8; i++)
            axii += sf::Joystick::hasAxis(0, sf::Joystick::Axis(i));

        log("axis count: " + std::to_string(axii));

        log(sf::Joystick::getIdentification(0).name);

        return;
    }else{
        log("no controller connected");
    }

    resize(windowWidth * windowScale, windowHeight * windowScale);
}


void deinitialize(){
    TLN_Deinit();
    free(framebuffer);
}

bool closed(){
    // frame_start = SDL_GetPerformanceCounter();
    // bool result = TLN_ProcessWindow();
    bool result = window.isOpen();

    // if(!result){
    //     deinitialize();
    // }

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
    return ((priority & 0x3) << 4) + (layer & 0x3);
}

void buildBG(int layer, int cbb, int sbb, int size, int prio, int mos){
    int p = buildLayerPrio(layer, prio);

    layerInfo[layer].priority = p;
    layerInfo[layer].sbb = sbb;
    layerInfo[layer].cbb = cbb;

    int a[4] = {-1,-1,-1,-1};

    for(int l = 0; l < 4; l++){
        for(int pos = 0; pos < 4; pos++){
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

        // log("layer " + std::to_string(a[pos]) + " prio " + std::to_string((layerInfo[a[pos]].priority >> 4) & 0x3));
        layerInfo[a[pos]].id = pos;
        TLN_SetTilemapTileset(tilemaps[layerInfo[a[pos]].sbb], tilesets[layerInfo[a[pos]].cbb]);
        TLN_SetLayerPriority(pos, (((layerInfo[a[pos]].priority >> 4) & 0x3) == 0));
        TLN_SetLayerTilemap(pos, tilemaps[layerInfo[a[pos]].sbb]);

        toggleBG(pos,layerInfo[pos].enabled);
    }

    enableBlend(blendInfo);
}

void vsync(){
    onVBlank();

    TLN_UpdateFrame(0);
    frameTexture.update((sf::Uint8*)framebuffer);
    sf::Sprite sprite;
    sprite.setTexture(frameTexture);
    sprite.setScale(windowScale,windowScale);
    sprite.setPosition(0,0);

    if(ENABLE_SHADERS && render){
        window.draw(sprite, &shader);

        sf::Color c = sf::Color::Black;

        c.a = 60;

        for(int i = 0; i < SCREEN_HEIGHT; i++){
            sf::RectangleShape rectangle(sf::Vector2f(SCREEN_WIDTH * windowScale,windowScale/2.0));

            rectangle.setPosition(sf::Vector2f(0,i * windowScale + windowScale / 2.0));

            rectangle.setFillColor(c);

            window.draw(rectangle);

            // sf::Vertex line[2];

            // line[0].position = sf::Vector2f(0,i * windowScale + (windowScale - 1));
            // line[1].position = sf::Vector2f(SCREEN_WIDTH * windowScale,i * windowScale + (windowScale - 1));

            // line[0].color = c;
            // line[1].color = c;

            // window.draw(line,2,sf::Lines);
        }

        // for(int i = 0; i < SCREEN_WIDTH; i++){
        //     sf::Vertex line[2];

        //     line[0].position = sf::Vector2f(i * windowScale + (windowScale),0);
        //     line[1].position = sf::Vector2f(i * windowScale + (windowScale),SCREEN_HEIGHT * windowScale);

        //     line[0].color = c;
        //     line[1].color = c;

        //     window.draw(line,2,sf::Lines);
        // }

    }else{
        if(render)
            window.draw(sprite);
    }

    window.display();

    // TLN_DrawFrame(0);
    sf::Event event;
    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed){
            window.close();
            deinitialize();

            exit(0);
        }else if(event.type == sf::Event::Resized){
            while(event.size.height / windowScale > 160 * 2){
                windowScale++;
            }
            while(windowScale > 1 && event.size.height / windowScale < 160){
                windowScale--;
            }

            resize(event.size.width, event.size.height);

        }else if(event.type == sf::Event::KeyPressed){
            if(event.key.code == sf::Keyboard::F11){
                isFullscreen = !isFullscreen;

                if(isFullscreen){
                    sf::VideoMode mode = sf::VideoMode::getDesktopMode();

                    window.create(mode, "Apotris PC",sf::Style::Fullscreen);
                }else{
                    window.create(sf::VideoMode(windowWidth * windowScale, windowHeight * windowScale), "Apotris PC");
                }

            }
        }
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)){
        window.setFramerateLimit(2000);

        if(currentSong >= 0)
            musicArray[currentSong].setPitch(3);
    }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)){
        window.setFramerateLimit(5);
    }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)){
        window.setFramerateLimit(1);
    }else{
        window.setFramerateLimit(60);

        if(currentSong >= 0)
            musicArray[currentSong].setPitch(1);
    }

    auto it = soundList.begin();

    while(it != soundList.end()){
        if((*it)->getStatus() == sf::Sound::Stopped){
            delete *it;
            soundList.erase(it++);
        }else{
            it++;
        }
    }

    if(currentSong >= 0 && musicArray[currentSong].getStatus() == sfmod::Mod::Stopped && musicVolume / 2 != 0){
        if(savefile->settings.cycleSongs){
            log("next");
            playNextSong();
        }else{
            log("loop");
            musicArray[currentSong].stop();
            musicArray[currentSong].play();
        }
    }

    // window.draw();

    // Uint64 end = SDL_GetPerformanceCounter();

    // float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    // // Cap to 60 FPS
    // SDL_Delay(floor(16.666f - elapsedMS));
}

void showSprites(int n ){
    // oam_init(oam_mem,128);
    // OBJ_ATTR* dest = (OBJ_ATTR*) MEM_OAM;

    std::list<int> spriteList;

    int sum = 0;
    for(int i = 4; i >= 1; i--){
        for(int counter = 127; counter >= 0; counter--){
            OBJ_ATTR *sprite = &obj_buffer[counter];
            if(sprite->enabled && sprite->priority == i && sprite->id >= 0 && sprite->id < 128){
                spriteList.push_back(sprite->id);
                sum++;
            }
        }
    }

    for(int i = 0; i < 128; i++){
        TLN_DisableSprite(i);
    }

    int counter = 0;
    sum = 0;
    for(auto const & id : spriteList){
        OBJ_ATTR *sprite = &obj_buffer[id];

        if(!sprite->enabled){
            counter++;
            continue;
        }

        sum++;

        // TLN_SpriteData d;
        spriteData[counter].x = (sprite->tile_start) * 8;
        spriteData[counter].y = 0;
        spriteData[counter].w = sprite->sx;
        spriteData[counter].h = sprite->sy;

        TLN_SetSpriteSet(counter, spriteset);
        TLN_SetSpritesetData(spriteset, counter, &spriteData[counter], 0,0);
        TLN_SetSpritePicture(counter, counter);
        TLN_SetSpritePalette(counter, palettes[sprite->palette + 16]);
        // TLN_SetSpriteFlags(counter, (FLAG_ROTATE));
        if(sprite->affine && !sprite->rotate){
            TLN_SetSpritePivot(counter, 0.5, 0.5);
            log(sprite->scalex);
            TLN_SetSpriteScaling(counter, sprite->scalex, sprite->scaley);
            float scaleOffset = (1+sprite->doubleSize)/2.0;
            TLN_SetSpritePosition(counter, sprite->x + offsetx + sprite->sx * scaleOffset, sprite->y + offsety + sprite->sy * scaleOffset);
        }else if (sprite->affine && sprite->rotate){
            TLN_SetSpritePivot(counter, 0, 0);
            // TLN_SetSpriteScaling(counter, sprite->scalex, sprite->scaley);
            TLN_SetSpritePosition(counter, sprite->x + offsetx, sprite->y + offsety);
            TLN_ResetSpriteScaling(counter);
        }else{
            TLN_SetSpritePosition(counter, sprite->x + offsetx, sprite->y + offsety);
            TLN_SetSpritePivot(counter, 0.0, 0.0);
            TLN_SetSpriteScaling(counter, 1.0, 1.0);
        }

        if(!(blendSprites || sprite->blend))
            TLN_SetSpriteBlendMode(counter, BLEND_NONE , 0);
        else
            TLN_SetSpriteBlendMode(counter, (blendSprites)?BLEND_CUSTOM:BLEND_MIX75 , 0);
            // TLN_SetSpriteBlendMode(counter, (blendSprites)?BLEND_MIX25:BLEND_MIX75 , 0);

        // TLN_SetSpriteBlendMode(counter, BLEND_MIX75, 0);
        TLN_SetSpriteFlags(counter, (sprite->flipx * FLAG_FLIPX) + (sprite->flipy * FLAG_FLIPY) + ((sprite->priority == 1) * FLAG_PRIORITY));
        // TLN_SetSpriteFlags(counter, (sprite->flipx * FLAG_FLIPX) + (sprite->flipy * FLAG_FLIPY) + (sprite->rotate * FLAG_ROTATE) + ((sprite->priority == 1) * FLAG_PRIORITY));

        counter++;
    }
}

void loadPalette(int palette, int index, const void * src, int count){
    for(int i = 0; i < (int) count; i++){
        u16 r = (u16) ((u16 *) src)[i];
        ColorSplit c = ColorSplit(r);
        int p = palette + i / 16;

        TLN_SetPaletteColor(palettes[p], index + i % 16, c.r, c.g, c.b);
    }

    if(palette == 0 && index == 0 && !gradientEnabled){
        u16 r = (u16) ((u16 *) src)[0];
        ColorSplit c = ColorSplit(r);

        TLN_SetBGColor(c.r, c.g, c.b);
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
    for(int i = 0; i < TILEMAP_SIZE; i++){
        for(int j = 0; j < TILEMAP_SIZE; j++){
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
    sprite->y = y + spriteVOffset;
};

void sprite_set_attr(OBJ_ATTR* sprite, int shape, int size, int tile_start, int palette, int priority){
    sprite->sx = sizeTable[shape][size][0];
    sprite->sy = sizeTable[shape][size][1];

    sprite->tile_start = tile_start;
    sprite->palette = palette;
    sprite->priority = priority + 1;
    sprite->affine = false;
    sprite->doubleSize = false;
    sprite->flipx = false;
    sprite->flipy = false;
    sprite->rotate = false;
    sprite->scalex = 1;
    sprite->scaley = 1;
    sprite->blend = false;
};

void setLayerScroll(int layer, int x, int y){
    if(!layerInfo[layer].enabled)
        return;

    TLN_SetLayerPosition(layerInfo[layer].id, x - offsetx, y - offsety);
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

void loadSpriteTilesPartial(int index, const void *src, int tx, int ty, int lengthX, int lengthY, int rowLength){
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
    previousKeys = currentKeys;
    currentKeys.clear();

    if(!window.hasFocus()){
        return;
    }

    // if(sf::Joystick::isConnected(0)){

    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 1)) * KEY_A;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 0)) * KEY_B;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 5)) * KEY_R;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 4)) * KEY_L;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 7)) * KEY_START;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 6)) * KEY_SELECT;

    //     float pos = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis(7));
    //     if(pos < 0)
    //         __key_curr |= (1) * KEY_UP;
    //     else if (pos > 0)
    //         __key_curr |= (1) * KEY_DOWN;

    //     pos = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis(6));
    //     if(pos < 0)
    //         __key_curr |= (1) * KEY_LEFT;
    //     else if (pos > 0)
    //         __key_curr |= (1) * KEY_RIGHT;

    // }

    for(int i = 0; i < sf::Keyboard::KeyCount; i++){
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key(i)) && sf::Keyboard::Key(i) != sf::Keyboard::Tab)
            currentKeys.push_back(sf::Keyboard::Key(i));
    }
}

void enableBlend(int info){

    int top = info & 0x3f;
    // int bot = (info >> 8) & 0x3f;
    int bot = 0;
    // int mode = (info >> 6) & 0x3;

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

    blendSprites = (top >> 4) & 0b1;

    blendInfo = info;
}

void setMusicTempo(int tempo){

}

void setMusicVolume(int volume){
    musicVolume = volume;

    if(currentSong != -1){
        int f = volume / 4;

        if(f){
            musicArray[currentSong].setMasterVolume(f);
            if(musicArray[currentSong].getStatus() != sfmod::Mod::Playing)
                musicArray[currentSong].play();
        }else{
            musicArray[currentSong].pause();
        }
    }
}

void setSFXVolume(int volume){
    sfxVolume = volume;
}

void startSong(int song, bool loop){
    if(song == currentSong && musicArray[currentSong].getStatus() == sfmod::Mod::Playing)
        return;

    if(currentSong != -1)
        musicArray[currentSong].stop();

    musicArray[song].play();
    currentSong = song;
}

u32 Sqrt(u32 n){
    return std::sqrt(n);
}

void sfx(int n){
    sf::Sound * sound = new sf::Sound();
    sound->setBuffer(sfxBuffers[n]);
    sound->play();
    sound->setVolume(savefile->settings.sfxVolume * sfxVolumeMultiplier);

    soundList.push_back(sound);
}

void sfxRate(int n, float rate){
    sf::Sound * sound = new sf::Sound();
    sound->setBuffer(sfxBuffers[n]);
    sound->play();
    sound->setVolume(savefile->settings.sfxVolume * sfxVolumeMultiplier);

    sound->setPitch(rate);

    soundList.push_back(sound);

    // sfx(sound);
}

void setTiles(int tilemap, int index, int count, u32 value){
    if(count == 32 * 32)
        count = TILEMAP_SIZE * TILEMAP_SIZE;

    TLN_Tile t = new Tile();
    t->value = value;
    // t->index = 1;
    // t->flags = 0;

    for(int i = index; i < count+index; i++){
        TLN_SetTilemapTile(tilemaps[tilemap], (i / TILEMAP_SIZE), (i % TILEMAP_SIZE), t);
    }

    delete t;
}

void clearSprites(int n){
    for(int counter = 0; counter < n; counter++){
        sprite_hide(&obj_buffer[counter]);
    }
}

void setPaletteColor(int palette, int index, u16 color, int count){
    ColorSplit c = ColorSplit(color);
    if(palette == 0 && index == 0)
        TLN_SetBGColor(c.r,c.g,c.b);

    for(int i = 0; i < count; i++){
        TLN_SetPaletteColor(palettes[palette], index+i, c.r, c.g, c.b);
    }
}

void clearSpriteTiles(int index, int lengthX, int lengthY){
    uint8_t tmp[8 * 8];
    memset(tmp,0,8 * 8);


    TLN_SpriteData d;
    memcpy(d.name,"clear",4);
    d.w = 8;
    d.h = 8;

    for(int i = 0; i < lengthY; i++){
        d.y = 8 * i;
        for(int j = 0; j < lengthX; j++){
            d.x = 8 * (j + index);

            TLN_SetSpritesetData(spriteset, 127, &d, &tmp, 8);
        }
    }

    // TLN_SetSpritesetData(spriteset, 0, &d, &tmp, 8);

}

void sprite_enable_mosaic(OBJ_ATTR *sprite){

}

void sprite_set_size(OBJ_ATTR *sprite, FIXED size, int aff_id){
    float n = fx2float(size);
    if(size == 0 || n == 0)
        return;

    float scale = 1/(n);

    sprite->scalex = scale;
    sprite->scaley = scale;
}

void sprite_rotscale(OBJ_ATTR *sprite, FIXED sizex, FIXED sizey, int angle, int aff_id){
    // sprite->affine = false;
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
    sprite->doubleSize = doubleSize;
}

void sprite_enable_flip(OBJ_ATTR *sprite, bool flipX, bool flipY){
    sprite->flipx = flipX;
    sprite->flipy = flipY;
}

extern "C" {

void posprintf(char * buff, const char * str, ...){

    va_list args;
    va_start(args,str);
    vsnprintf(buff, 64, str, args);
    va_end(args);
}

}

void stopSong(){
    if(currentSong == -1)
        return;

    musicArray[currentSong].stop();

    currentSong = -1;
};

void pauseSong(){
    if(currentSong == -1)
        return;

    musicArray[currentSong].pause();
};

void resumeSong(){
    if(currentSong == -1 || savefile->settings.volume == 0)
        return;

    musicArray[currentSong].play();
};

void color_fade_palette(int palette, int index, const COLOR * src, COLOR color, int count, u32 alpha){
    u16 temp[count];

    color_fade(temp,src,color,count,alpha);

    int remaining = index + count;

    int counter = 0;
    while(remaining > 0){

        int m = remaining % 16;
        if(m){

            loadPalette(palette, index, temp, m);

            remaining -= m;
        }else{
            loadPalette(palette + counter, 0, &temp[count-remaining], 16);
            remaining -= 16;
        }

        counter++;
    }
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
}

void color_blend(COLOR *dst, const COLOR *srca, const COLOR *srcb, uint nclrs, u32 alpha) {
    if(!nclrs)
        return;

    int ii;
    u32 clra, clrb, clr;
    u32 parta, partb, part;
    const u32 rbmask= (RED_MASK|BLUE_MASK), gmask= GREEN_MASK;
    const u32 rbhalf= 0x4010, ghalf= 0x0200;

    for(ii=0; ii<nclrs; ii++)
    {
        clra= srca[ii];
        clrb= srcb[ii];

        // Red and blue
        parta= clra & rbmask;
        partb= clrb & rbmask;
        part = (partb-parta)*alpha + parta*32 + rbhalf;
        clr  = (part/32 ) & rbmask;

        // Green
        parta= clra & gmask;
        partb= clrb & gmask;
        part = (partb-parta)*alpha + parta*32 + ghalf;
        clr |= (part/32 ) & gmask;

        dst[ii]= clr;
    }
}

void color_adj_MEM(COLOR *dst, const COLOR *src, u32 count, u32 bright){
    u32 ii, clr;
    int rr, gg, bb;

    bright >>= 3;
    for(ii=0; ii < count; ii++)
    {
        clr= src[ii];

        rr= ((clr    )&31) + bright;
        gg= ((clr>>5 )&31) + bright;
        bb= ((clr>>10)&31) + bright;

        dst[ii]= RGB15(bf_clamp(rr,5), bf_clamp(gg,5), bf_clamp(bb,5));
    }

}

void color_adj_brightness(int palette, int index, const COLOR *src, u32 count, u32 bright){
    COLOR dst[count];

    color_adj_MEM(dst, src, count, bright);

    loadPalette(palette, index, dst, count);
}

void clr_grayscale(COLOR *dst, const COLOR *src, u32 nclrs){
    u32 ii;
    u32 clr, gray, rr, gg, bb;

    for(ii=0; ii<nclrs; ii++)
    {
        clr= *src++;

        // Do RGB conversion in .8 fixed point
        rr= ((clr    )&31)*0x4C;    // 29.7%
        gg= ((clr>> 5)&31)*0x96;    // 58.6%
        bb= ((clr>>10)&31)*0x1E;    // 11.7%
        gray= (rr+gg+bb+0x80)>>8;

        *dst++= RGB15(gray, gray, gray);
    }
}

void clr_rgbscale(COLOR *dst, const COLOR *src, uint nclrs, COLOR clr){
    uint ii;
    u32 rr, gg, bb, scale, gray;

    // Normalize target color
    rr= (clr    )&31;
    gg= (clr>> 5)&31;
    bb= (clr>>10)&31;

    scale= max(max(rr, gg), bb);
    if(scale == 0)    // Can't scale to black : use gray instead
    {
        clr_grayscale(dst, src, nclrs);
        return;
    }

    scale = ((1 << 16)/scale) & 0xffff;

    rr *= scale;        // rr, gg, bb -> 0.16f
    gg *= scale;
    bb *= scale;

    for(ii=0; ii<nclrs; ii++)
    {
        clr= *src++;

        // Grayscaly in 5.8f
        gray  = ((clr    )&31)*0x4C;    // 29.7%
        gray += ((clr>> 5)&31)*0x96;    // 58.6%
        gray += ((clr>>10)&31)*0x1E;    // 11.7%

        // Match onto color vector
        dst[ii]= RGB15(rr*gray>>24, gg*gray>>24, bb*gray>>24);
    }
};

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
    // log(std::to_string(index) + " " + std::to_string(count));
    for(int i = index; i < count+index; i++){
        setTile(tilemap, i%32, i/32, 0);
    }
}

void stopDMA(){};
void setMosaic(int sx, int sy){
    for(int i = 0; i < 4; i++){
        if(!sx || !sy)
            TLN_DisableLayerMosaic(i);
        else
            TLN_SetLayerMosaic(i, sx, sy);

        // TLN_SetLayerMosaic(i, 1, 2);
        // TLN_SetLayerClip(i, 0, 0, 0, 0);

    }
};

void sleep(){};
void rumble_set_state(int n){};

// static uint64_t frame_start = 0;
// static uint64_t frame_end = 0;

void onVBlank(){
    if (canDraw) {
        canDraw = 0;

        scene->draw();
    }

    frameCounter++;

    // frame_end = SDL_GetPerformanceCounter();

    // float elapsedMS = (frame_end - frame_start) / (float) SDL_GetPerformanceFrequency() * 1000.0f;

    // int delay = (std::floor(16.666f - elapsedMS));

    // if(frame_start > 0 && delay > 0)
    //     SDL_Delay(delay);

    // frame_start = SDL_GetPerformanceCounter();


    // int count = 0;
    // while(SDL_PollEvent(&test_event)){
    //     switch(test_event.type){
    //         case SDL_MOUSEBUTTONDOWN:
    //         log("test");
    //         break;
    //     }
    //     if(++count > 100){
    //         log("broke");
    //         break;
    //     }
    // }
};

void onHBlank(int line){
    if(!gradientEnabled){
        return;
    }

    ColorSplit c = ColorSplit(gradientTable[line]);
    TLN_SetBGColor(c.r, c.g, c.b);
}

void loadSavefile(){
    // memset32(savefile, 0 , sizeof(Save)/4);

    std::ifstream input("Apotris.sav",std::ios::binary | std::ios::in);

    if(savefile == nullptr)
        savefile = new Save();

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

    const int saveSize = 1 << 15;

    char temp[saveSize];

    memset32(temp, 0, saveSize / 4);
    memcpy32(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

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
    float r = (float)angle * ((3.1459) / 256);
    int adj = (int) ((float)(1 << 12) * (sin(r)));
    return (adj & 0xffff);
}

COLOR rgb8to5(u32 *c){
    int b = (*c >> (16 + 3)) & 0x1f;
    int g = (*c >> ( 8 + 3)) & 0x1f;
    int r = (*c >> ( 0 + 3)) & 0x1f;

    return r | (g << 5) | (b << 10);
}

void savePalette(COLOR *dst){
    for(int i = 0; i < 32; i++)
        for(int j = 0; j < 16; j++)
            *dst++ = rgb8to5((u32*) TLN_GetPaletteData(palettes[i], j));
}

void color_fade_fast(int palette, int index, const COLOR * src, COLOR color, int count, u32 alpha){
    u16 temp[count];

    color_fade(temp,src,color,count,alpha);

    if(count > 16){
        int n = count/16;

        for(int i = 0; i < (n); i++)
            loadPalette(palette + i, index, &temp[i * 16], 16);
    }else{
        loadPalette(palette, index, temp, count);
    }
}

void mirrorPalettes(int index, int count){
    for(int i = 0; i < count; i++){
        COLOR temp[16];

        temp[0] = 0;

        for(int j = index; j < 16; j++)
            temp[j] = rgb8to5((u32 *)TLN_GetPaletteData(palettes[i+16], j));

        loadPalette(i, 0, temp, 16);
        // for(int j = 0; j < 16; j++){
        //     setPaletteColor(i, j, rgb8to5((u32*) TLN_GetPaletteData(palettes[i + 16], j)), 1);
        // }
    }
}

void sprite_enable_blend(OBJ_ATTR * sprite){
    sprite->blend = true;
}

void addColorToPalette(int palette, int index, COLOR color, int count){

    COLOR dst[count];

    for(int i = 0; i < count; i++)
        dst[i] = rgb8to5((u32*) TLN_GetPaletteData(palettes[palette], index + i)) + color;

    loadPalette(palette, index, dst, count);
}

uint8_t customBlend(uint8_t a, uint8_t b){
    return (a + b + b + b + b) /5;
}

void toggleRendering(bool state){
    render = state;
}

void resize(int width, int height){
    sf::FloatRect visibleArea(sf::Vector2f(0,0), sf::Vector2f(width, height));
    sf::View view(visibleArea);
    view.setCenter(256 * windowScale,256 * windowScale);
    window.setView(view);

    rowStart = (512 - (height/windowScale))/2;
    rowEnd = (512 + (height/windowScale))/2;

    if(savefile != nullptr)
        setGradient(savefile->settings.backgroundGradient);
}

std::vector<std::string> keyToString = {
    "A",        //!< The A key
    "B",            //!< The B key
    "C",            //!< The C key
    "D",            //!< The D key
    "E",            //!< The E key
    "F",            //!< The F key
    "G",            //!< The G key
    "H",            //!< The H key
    "I",            //!< The I key
    "J",            //!< The J key
    "K",            //!< The K key
    "L",            //!< The L key
    "M",            //!< The M key
    "N",            //!< The N key
    "O",            //!< The O key
    "P",            //!< The P key
    "Q",            //!< The Q key
    "R",            //!< The R key
    "S",            //!< The S key
    "T",            //!< The T key
    "U",            //!< The U key
    "V",            //!< The V key
    "W",            //!< The W key
    "X",            //!< The X key
    "Y",            //!< The Y key
    "Z",            //!< The Z key
    "0",         //!< The 0 key
    "1",         //!< The 1 key
    "2",         //!< The 2 key
    "3",         //!< The 3 key
    "4",         //!< The 4 key
    "5",         //!< The 5 key
    "6",         //!< The 6 key
    "7",         //!< The 7 key
    "8",         //!< The 8 key
    "9",         //!< The 9 key
    "Esc",       //!< The Escape key
    "LCtrl",     //!< The left Control key
    "LShift",       //!< The left Shift key
    "LAlt",         //!< The left Alt key
    "LWin",      //!< The left OS specific key: window (Windows and Linux), apple (macOS), ...
    "RCtrl",     //!< The right Control key
    "RShift",       //!< The right Shift key
    "RAlt",         //!< The right Alt key
    "RWin",      //!< The right OS specific key: window (Windows and Linux), apple (macOS), ...
    "Menu",         //!< The Menu key
    "[",     //!< The [ key
    "]",     //!< The ] key
    ";",    //!< The ; key
    ",",        //!< The , key
    ".",       //!< The . key
    "'",   //!< The ' key
    "/",        //!< The / key
    "\\",    //!< The \ key
    "`",        //!< The ` key
    "=",        //!< The = key
    "-",       //!< The - key (hyphen)
    "Space",        //!< The Space key
    "Enter",        //!< The Enter/Return keys
    "Bckspc",    //!< The Backspace key
    "Tab",          //!< The Tabulation key
    "PgUp",       //!< The Page up key
    "PgDn",     //!< The Page down key
    "End",          //!< The End key
    "Home",         //!< The Home key
    "Insert",       //!< The Insert key
    "Del",       //!< The Delete key
    "+",          //!< The + key
    "-",     //!< The - key (minus, usually from numpad)
    "*",     //!< The * key
    "/",       //!< The / key
    "Left",         //!< Left arrow
    "Right",        //!< Right arrow
    "Up",           //!< Up arrow
    "Down",         //!< Down arrow
    "Num0",      //!< The numpad 0 key
    "Num1",      //!< The numpad 1 key
    "Num2",      //!< The numpad 2 key
    "Num3",      //!< The numpad 3 key
    "Num4",      //!< The numpad 4 key
    "Num5",      //!< The numpad 5 key
    "Num6",      //!< The numpad 6 key
    "Num7",      //!< The numpad 7 key
    "Num8",      //!< The numpad 8 key
    "Num9",      //!< The numpad 9 key
    "F1",           //!< The F1 key
    "F2",           //!< The F2 key
    "F3",           //!< The F3 key
    "F4",           //!< The F4 key
    "F5",           //!< The F5 key
    "F6",           //!< The F6 key
    "F7",           //!< The F7 key
    "F8",           //!< The F8 key
    "F9",           //!< The F9 key
    "F10",          //!< The F10 key
    "F11",          //!< The F11 key
    "F12",          //!< The F12 key
    "F13",          //!< The F13 key
    "F14",          //!< The F14 key
    "F15",          //!< The F15 key
    "Pause",        //!< The Pause key
};

void initRumble(){}

#endif
