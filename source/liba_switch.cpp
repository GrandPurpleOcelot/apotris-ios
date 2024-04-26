#ifdef SWITCH

#include "liba_switch.h"
#include <list>
#include <fstream>

#include "def.h"
#include "switch.h"

#include "nanotime.h"

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

int rowStart = 0;
int rowEnd = SCREEN_HEIGHT;

#define FPS_TARGET 60

double clock_timer = 0;

static uint64_t frame_start = nanotime_now();

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

SDL_Window* window;
SDL_Renderer * renderer;

bool render = true;

std::list<uint32_t> currentlyPressed;

int screenWidth = 1280;
int screenHeight = 720;

float windowScale = 4;

nanotime_step_data stepper;

void windowInit(){
    // Initialize SDL
    romfsInit();
    // chdir("romfs:/");

    window = NULL;
    window = SDL_CreateWindow(
        "Apotris Switch",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
        );

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);

    loadAudio("romfs:/");

    nanotime_step_init(&stepper, (uint64_t) (NANOTIME_NSEC_PER_SEC / FPS_TARGET), nanotime_now_max(),nanotime_now,nanotime_sleep);
}

void key_poll(){
    previousKeys = currentKeys;
    currentKeys.clear();

    SDL_Event event;

    while( SDL_PollEvent( &event ) ){
        uint32_t key = 0;
        auto it = currentlyPressed.begin();
        bool found = false;

        switch( event.type ){
        case SDL_JOYBUTTONDOWN:
            key = event.jbutton.button;
            it = std::find(currentlyPressed.begin(),currentlyPressed.end(),key);
            found = (it != currentlyPressed.end());
            if(!found)
                currentlyPressed.push_back(key);
            break;
        case SDL_JOYBUTTONUP:
            key = event.jbutton.button;
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
}

uint32_t key_hit(uint32_t key){
    if(key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),key);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),key);
    bool curr = (it != currentKeys.end());

    return (!prev && curr);
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
    const int in_width = 512;
    const int in_height = 512;

    SDL_Surface *img = SDL_CreateRGBSurfaceFrom(
        framebuffer,
        in_width,
        in_height,
        32,
        in_width * 4,
        0x0000ff,
        0x00ff00,
        0xff0000,
        0xff000000
    );

    // delete [] newBuffer;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_FreeSurface(img);

    SDL_RenderClear(renderer);

    SDL_Rect rect = {
        0,
        0,
        in_width,
        in_height,
    };

    const int w = in_width * windowScale;
    const int h = in_height * windowScale;

    SDL_Rect dest_rect = {
        -(w - screenWidth)/2,
        -(h - screenHeight)/2,
        w,
        h,
    };

    if(render){
        SDL_RenderCopy(renderer,texture,&rect,&dest_rect);
    }

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    // Cap to 60 FPS
    nanotime_step(&stepper);

    Uint64 end = SDL_GetPerformanceCounter();

    float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    if(elapsedMS > 100){
        nanotime_step_init(&stepper, (uint64_t) (NANOTIME_NSEC_PER_SEC / FPS_TARGET), nanotime_now_max(),nanotime_now,nanotime_sleep);
    }

    fps = 1000.0f / elapsedMS;

    frame_start = SDL_GetPerformanceCounter();
}

void refreshWindowSize(){
    if(savefile != nullptr){
        if(savefile->settings.zoom > -1){
            windowScale = 1 + (float) savefile->settings.zoom / 10;
        }else{
            if(savefile->settings.integerScale){
                windowScale = (int)windowScale;

                while (windowScale > 0 && screenHeight / windowScale > 160 * 2) {
                    windowScale++;
                }
                while (windowScale > 1 && screenHeight / windowScale < 160) {
                    windowScale--;
                }
            }else{
                windowScale = screenHeight / 200.0;
            }
        }
    }

    if(windowScale <= 0)
        return;

    rowStart = (SCREEN_HEIGHT - (screenHeight/windowScale))/2;
    rowEnd = (SCREEN_HEIGHT + (screenHeight/windowScale))/2;

    if(rowStart < 0)
        rowStart = 0;

    if(rowEnd > SCREEN_HEIGHT)
        rowEnd = SCREEN_HEIGHT;

    if(savefile != nullptr)
        setGradient(savefile->settings.backgroundGradient);
}

bool closed(){
    songEndHandler();

    return true;
}

void toggleRendering(bool r){
    render = r;
}

void initRumble(){}

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
    std::ofstream output("Apotris.sav",std::ios::binary | std::ios::out);

    char * dst = (char * ) savefile;

    const int saveSize = 1 << 15;

    char temp[saveSize];

    memset32_fast(temp, 0, saveSize / 4);
    memcpy32_fast(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

    if(!output){
        log("Error when trying to write save.");
        return;
    }

    output.close();
}

std::map<int,std::string> keyToString = {
{0,"A"},
{1,"B"},
{2,"X"},
{3,"Y"},
{6,"L"},
{7,"R"},
{8,"SL"},
{9,"SR"},
{10,"+"},
{11,"-"},
{12,"Left"},
{13,"Up"},
{14,"Right"},
{15,"Down"},
};

void quit(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    freeAudio();
    exit(0);
}

#endif
