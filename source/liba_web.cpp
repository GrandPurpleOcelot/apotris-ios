#ifdef WEB

#include "liba_web.h"
#include <list>
#include <fstream>

#include "SDL2/SDL.h"
#include "def.h"
#include <emscripten.h>

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

int rowStart = (240 - 240) / 2;
int rowEnd = rowStart + 240;
// int rowStart = 0;
// int rowEnd = 160;

#define FPS_TARGET 60

double clock_timer = 0;

SDL_Window* window;
SDL_Renderer * renderer;

void windowInit(){
    // Initialize SDL

    window = NULL;
    window = SDL_CreateWindow(
        "Apotris Web",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN
        );

    renderer = SDL_CreateRenderer(window, -1, 0);
}

std::list<uint32_t> currentlyPressed;

void key_poll(){
    previousKeys = currentKeys;
    currentKeys.clear();

    currentKeys = currentlyPressed;

    SDL_Event event;

    while( SDL_PollEvent( &event ) ){
        uint32_t key = 0;
        auto it = currentlyPressed.begin();
        bool found = false;

        switch( event.type ){
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            it = std::find(currentlyPressed.begin(),currentlyPressed.end(),key);
            found = (it != currentlyPressed.end());
            if(!found)
                currentlyPressed.push_back(key);
            break;
        case SDL_KEYUP:
            key = event.key.keysym.sym;
            it = std::find(currentlyPressed.begin(),currentlyPressed.end(),key);
            found = (it != currentlyPressed.end());
            if(found)
                currentlyPressed.remove(key);
            break;
        case SDL_QUIT:
            std::cout << "quitting!\n";
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            break;
        default:
            break;
        }
    }
}

uint32_t key_is_down(uint32_t key){
    if(key == KEY_FULL)
        return (!currentKeys.empty());

    auto it = currentKeys.begin();

    it = std::find(currentKeys.begin(),currentKeys.end(),key);

    return (it != currentKeys.end());

    return 0;
}

uint32_t key_hit(uint32_t key){
    if(key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty())? currentKeys.front() : 0;

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),key);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),key);
    bool curr = (it != currentKeys.end());

    return (!prev && curr);

    return 0;
}

uint32_t key_released(uint32_t key){
    if(key == KEY_FULL)
        return (!previousKeys.empty() && currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),key);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),key);
    bool curr = (it != currentKeys.end());

    return (prev && !curr);

    return 0;
}

void updateWindow(uint8_t* framebuffer){
    // // // Update the screen
    // // SDL_UpdateRect(screen, 0, 0, 0, 0);

    uint8_t * newBuffer = new uint8_t[640 * 480 * 4];

    const int in_width = 320;
    const int in_height = 240;

    uint32_t* out_ptr = (uint32_t *) newBuffer;
    uint32_t* in_ptr = (uint32_t *) framebuffer;
    int in_pitch = in_width;
    int out_pitch = 640;

    in_ptr += (((in_height - 240) / 2)) * in_pitch;
    in_ptr += (in_width - 320) / 2;

    for(int y = 0; y < 240; y++){
        uint32_t * line_ptr = in_ptr;

        for(int x = 0; x < 320; x++){
            uint32_t * out_line_ptr = out_ptr;
            uint32_t color = *line_ptr;

            *out_line_ptr = color;
            *(out_line_ptr + 1) = color;

            out_line_ptr += out_pitch;
            *out_line_ptr = color;
            *(out_line_ptr + 1) = color;

            out_ptr += 2;

            line_ptr++;
        }

        in_ptr += in_pitch;
        out_ptr += out_pitch;
    }

    SDL_Surface *img = SDL_CreateRGBSurfaceFrom(
        newBuffer,
        640,
        480,
        32,
        640 * 4,
        0x0000ff,
        0x00ff00,
        0xff0000,
        0xff000000
    );

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer,texture,NULL,NULL);
    SDL_RenderPresent(renderer);

    // Free surfaces
    SDL_FreeSurface(img);
    delete [] newBuffer;

    SDL_DestroyTexture(texture);
}

void refreshWindowSize(){

}

static uint64_t frame_start = SDL_GetPerformanceCounter();

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

bool closed(){

    // // Cap to 60 FPS
    // Uint64 end = SDL_GetPerformanceCounter();

    // float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    // SDL_Delay(floor(16.666f - elapsedMS));
    // frame_start = SDL_GetPerformanceCounter();

    // //show fps
    frame_time = SDL_GetTicks()-start_time;
    fps = (frame_time > 0)? 1000.0 / frame_time : 0.0;
    start_time = SDL_GetTicks();

    std::cout << "fps: " << fps << "\n";


    return true;
}

void sfx(int){

}

void startSong(int,bool){

}

void stopSong(){};

void resumeSong(){};

void stopMusicVolume(){};

void setMusicVolume(int){};

void setMusicTempo(int){};

void setSFXVolume(int){};

void toggleRendering(bool){};

void sfxRate(int,float){};

void pauseSong(){};
void initRumble(){};

void loadSavefile(){
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

#endif
