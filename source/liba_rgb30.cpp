#ifdef RGB

#include "liba_rgb30.h"
#include <list>
#include <fstream>

#include "soloud.h"
#include "soloud_wav.h"
#include "def.h"
void loadAudio();
void freeAudio();

Songs songs;

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

// int rowStart = (288 - 288) / 2;
// int rowEnd = rowStart + 288;
int rowStart = 0;
int rowEnd = 360;

#define FPS_TARGET 60

double clock_timer = 0;

static uint64_t frame_start = SDL_GetPerformanceCounter();

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

SDL_Window* window;
SDL_Renderer * renderer;
SDL_GameController* controller;

bool render = true;

std::list<uint32_t> currentlyPressed;

SoLoud::Soloud gSoloud;

SoLoud::Wav soundEffects[SFX_COUNT];

int screenWidth = 480;
int screenHeight = 320;
float windowScale = 3;

void windowInit(){
    // Initialize SDL

    window = NULL;
    // window = SDL_CreateWindow(
    //     "Apotris PortMaster",
    //     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    //     screenWidth, screenHeight,
    //     SDL_WINDOW_SHOWN
    //     );

    // SDL_Init(SDL_INIT_VIDEO);
    // SDL_DisplayMode DM;
    // SDL_GetCurrentDisplayMode(0, &DM);
    // screenWidth = DM.w;
    // screenHeight = DM.h;

    window = SDL_CreateWindow(
        "Apotris PortMaster",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screenWidth, screenHeight,
        SDL_WINDOW_SHOWN
        );

    renderer = SDL_CreateRenderer(window, -1, 0);

    // SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    // if(SDL_NumJoysticks() > 0){
    //     controller = SDL_GameControllerOpen(0);
    // }

    gSoloud.init();
    loadAudio();
}

void loadAudio(){
    for(int i = 0; i < SFX_COUNT; i++){
        std::string str = (std::string) SoundEffectPaths[i];

        soundEffects[i].load(str.c_str());
    }
}

void freeAudio(){
}

void key_poll(){
    previousKeys = currentKeys;
    currentKeys.clear();

    SDL_Event event;

    while( SDL_PollEvent( &event ) ){
        switch( event.type ){
        case SDL_QUIT:
            quit();
            break;
        default:
            break;
        }
    }

    if(controller && SDL_GameControllerGetAttached(controller)){
        for(int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++){
            if(SDL_GameControllerGetButton(controller,static_cast<SDL_GameControllerButton>(i))){
                currentKeys.push_back(i);
            }
        }
    }

    // currentKeys = currentlyPressed;
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
    const int in_width = 360;
    const int in_height = 360;

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

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_FreeSurface(img);

    const int x = (in_width - screenWidth / windowScale) / 2;
    const int y = (in_height - screenHeight / windowScale) / 2;
    const int w = screenWidth / windowScale;
    const int h = screenHeight / windowScale;

    SDL_Rect rect = {x,y,w,h};

    SDL_RenderClear(renderer);

    if(render)
        SDL_RenderCopy(renderer,texture,&rect,NULL);

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    // Cap to 60 FPS
    Uint64 end = SDL_GetPerformanceCounter();

    float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    float delay = max(floor(16.666f - elapsedMS),0);
    SDL_Delay(delay);
    frame_start = SDL_GetPerformanceCounter();
}

void refreshWindowSize(){
    if(savefile != nullptr){
        if(savefile->settings.zoom > -1){
            windowScale = 1 + (float) savefile->settings.zoom / 10;
        }else{
            if(savefile->settings.integerScale){
                windowScale = (int)windowScale;

                while (screenHeight / windowScale > 160 * 2) {
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

    rowStart = (512 - (screenHeight/windowScale))/2;
    rowEnd = (512 + (screenHeight/windowScale))/2;

    if(rowStart < 0)
        rowStart = 0;

    if(rowEnd > 512)
        rowEnd = 512;

    if(savefile != nullptr)
        setGradient(savefile->settings.backgroundGradient);
}

bool closed(){
    // //show fps
    // frame_time = SDL_GetTicks()-start_time;
    // fps = (frame_time > 0)? 1000.0 / frame_time : 0.0;
    // start_time = SDL_GetTicks();

    // std::cout << "fps: " << fps << "\n";


    return true;
}

void sfx(int n){
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    gSoloud.play(soundEffects[n]);
}

void startSong(int,bool){

}

void stopSong(){};

void resumeSong(){};

void setMusicVolume(int){};

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

    memset32(temp, 0, saveSize / 4);
    memcpy32(temp, dst, sizeof(Save) / 4);

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
    gSoloud.deinit();
    exit(0);
}

#endif
