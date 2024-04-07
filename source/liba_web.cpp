#ifdef WEB

#include "liba_web.h"
#include <list>
#include <fstream>

#include <emscripten.h>
#include <emscripten/html5.h>
#include "soloud.h"
#include "soloud_wav.h"
#include "def.h"

void loadAudio();
void freeAudio();
void resizeWindow(int width, int height);

Songs songs;

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

std::list<uint32_t> currentlyPressed;

int rowStart = (240 - 240) / 2;
int rowEnd = rowStart + 240;
// int rowStart = 0;
// int rowEnd = 160;

#define FPS_TARGET 60

double clock_timer = 0;

SDL_Window* window;
SDL_Renderer * renderer;

bool render = true;

static uint64_t frame_start = SDL_GetPerformanceCounter();

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

SoLoud::Wav soundEffects[SFX_COUNT];

int sfxVolume = 1024;
int musicVolume = 1024;

int windowWidth = 640;
int windowHeight = 480;

float zoom = 0;

SoLoud::Soloud gSoloud;

EM_JS(int, getWindowWidth, (), {
  return window.innerWidth;
});

EM_JS(int, getWindowHeight, (), {
  return window.innerHeight;
});

int resizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData){
    windowWidth = getWindowWidth();
    windowHeight = getWindowHeight();

    resizeWindow(windowWidth, windowHeight);
    return 0;
}

void windowInit(){
    // Initialize SDL

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

    EM_ASM(
        // Make a directory other than '/'
        FS.mkdir('/offline');
        // Then mount with IDBFS type
        FS.mount(IDBFS, {}, '/offline');

        // Then sync
        FS.syncfs(true, function (err) {
            // Error
        });
    );

    emscripten_sleep(100);

    window = SDL_CreateWindow(
        "Apotris Web",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );

    resizeWindow(getWindowWidth(),getWindowHeight());

    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    gSoloud.init();

    loadAudio();

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,false,resizeCallback);
}

void loadAudio(){
    for(int i = 0; i < SFX_COUNT; i++){
        std::string str = "bin/wasm/" + (std::string) SoundEffectPaths[i];

        soundEffects[i].load(str.c_str());
    }
}

void freeAudio(){
}


void key_poll(){
    previousKeys = currentKeys;
    currentKeys.clear();

    // if(windowWidth != getWindowWidth() || windowHeight != getWindowHeight()){

    //     windowWidth = getWindowWidth();
    //     windowHeight = getWindowHeight();

    //     resizeWindow(windowWidth, windowHeight);
    // }

    SDL_Event event;

    while( SDL_PollEvent( &event ) ){
        uint32_t key = 0;
        auto it = currentlyPressed.begin();
        bool found = false;

        switch( event.type ){
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            if(key == SDLK_F11){
                // log("here!");
                // emscripten_request_fullscreen(document.documentElement,1);
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                break;
            }

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
            quit();
            break;
        default:
            break;
        }
    }

    currentKeys = currentlyPressed;
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

uint32_t key_first(){
    if(currentKeys.empty())
        return KEY_FULL - 1;

    for(auto key : currentKeys){
        auto it = previousKeys.begin();
        it = std::find(previousKeys.begin(),previousKeys.end(),key);
        bool prev = (it != previousKeys.end());

        if(!prev){
            return key;
        }
    }

    return KEY_FULL - 1;
}

void updateWindow(uint8_t* framebuffer){
    // // // Update the screen
    // // SDL_UpdateRect(screen, 0, 0, 0, 0);

    // uint8_t * newBuffer = new uint8_t[640 * 480 * 4];

    const int in_width = 512;
    const int in_height = 512;

    // uint32_t* out_ptr = (uint32_t *) newBuffer;
    // uint32_t* in_ptr = (uint32_t *) framebuffer;
    // int in_pitch = in_width;
    // int out_pitch = 640;

    // in_ptr += (((in_height - 240) / 2)) * in_pitch;
    // in_ptr += (in_width - 320) / 2;

    // for(int y = 0; y < 240; y++){
    //     uint32_t * line_ptr = in_ptr;

    //     for(int x = 0; x < 320; x++){
    //         uint32_t * out_line_ptr = out_ptr;
    //         uint32_t color = *line_ptr;

    //         *out_line_ptr = color;
    //         *(out_line_ptr + 1) = color;

    //         out_line_ptr += out_pitch;
    //         *out_line_ptr = color;
    //         *(out_line_ptr + 1) = color;

    //         out_ptr += 2;

    //         line_ptr++;
    //     }

    //     in_ptr += in_pitch;
    //     out_ptr += out_pitch;
    // }

    SDL_Surface *img = SDL_CreateRGBSurfaceFrom(
        framebuffer,
        in_width,
        in_height,
        32,
        in_width * 4,
        0x0000ff,
        0x00ff00,
        0xff0000,
        0
    );

    // delete [] newBuffer;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_Rect rect = {(in_width - windowWidth) / 2, (in_height - windowHeight) / 2, windowWidth, windowHeight};

    SDL_FreeSurface(img);

    SDL_RenderClear(renderer);

    if(render)
        SDL_RenderCopy(renderer,texture,&rect,NULL);

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    Uint64 end = SDL_GetPerformanceCounter();

    float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    SDL_Delay(floor(16.666f - elapsedMS));
    frame_start = SDL_GetPerformanceCounter();
}

void refreshWindowSize(){
    resizeWindow(getWindowWidth(),getWindowHeight());
}

bool closed(){
    emscripten_sleep(0);
    // // Cap to 60 FPS

    // //show fps
    // frame_time = SDL_GetTicks()-start_time;
    // fps = (frame_time > 0)? 1000.0 / frame_time : 0.0;
    // start_time = SDL_GetTicks();

    // std::cout << "fps: " << fps << "\n";


    return true;
}

void sfx(int n){
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    unsigned int s = gSoloud.play(soundEffects[n]);
    s + 1;
}

void startSong(int,bool){

}

void stopSong(){};

void resumeSong(){};

void setMusicVolume(int vol){
    musicVolume = vol;
}

void setMusicTempo(int){};

void toggleRendering(bool r){
    render = r;
}

void sfxRate(int n, float rate){
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    int s = gSoloud.play(soundEffects[n]);
    gSoloud.setRelativePlaySpeed(s,rate);
}

void pauseSong(){};
void initRumble(){};

void loadSavefile(){
    EM_ASM(
        var s = FS.open("/offline/Apotris.sav","a");
        FS.close(s);
    );

    std::ifstream input("/offline/Apotris.sav",std::ios::binary | std::ios::in);

    if(!input){
        log("Error when trying to load save.");
        return;
    }

    if(savefile == nullptr)
        savefile = new Save();

    char * src = (char * ) savefile;

    input.read(src,sizeof(Save));

    input.close();
}

void saveSavefile(){
    std::ofstream output("/offline/Apotris.sav",std::ios::binary | std::ios::out);

    if(!output){
        log("Error when trying to write save.");
        return;
    }

    char * dst = (char * ) savefile;

    const int saveSize = 1 << 15;

    char temp[saveSize];

    memset32_fast(temp, 0, saveSize / 4);
    memcpy32_fast(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

    output.close();

    EM_ASM(
        FS.syncfs(function (err) {
            // Error
        });
    );
    emscripten_sleep(100);
}

void quit() {
    freeAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    gSoloud.deinit();
}

void resizeWindow(int width, int height){
    if(savefile != nullptr){
        zoom = ((float) max(savefile->settings.zoom,10) / 20);
    }else{
        zoom = 1;
    }

    log("test:");
    log((int)(zoom * 100));

    emscripten_set_canvas_size(width, height);
    SDL_SetWindowSize(window, width, height);

    windowHeight = (int) 160 / zoom;
    windowWidth = ((float)width / height) * windowHeight;

    rowStart = (512 - (windowHeight))/2;
    rowEnd = (512 + (windowHeight))/2;
}

std::string stringFromKey(uint32_t key){
    return SDL_GetKeyName(key);
}

#endif
