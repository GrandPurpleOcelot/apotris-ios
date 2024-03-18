#ifdef PC

#include "liba_window.h"
#include <list>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <cstdio>

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_openmpt.h"
#include "def.h"

#ifdef GL
#include "shader.h"
#endif

#include "nanotime.h"

void loadAudio();
void freeAudio();

Songs songs;

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

// int rowStart = (288 - 288) / 2;
// int rowEnd = rowStart + 288;
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
SDL_GameController* controller;

bool fullscreen = false;

bool render = true;

std::list<uint32_t> currentlyPressed;

SoLoud::Soloud gSoloud;

SoLoud::Wav soundEffects[SFX_COUNT];

SoLoud::Openmpt *musicArray;

SoLoud::handle currentSongHandle;

std::vector<std::string> songList;

int screenWidth = 1280;
int screenHeight = 720;

float windowScale = 3;
int currentSong = -1;
int sfxVolume = 1024;
int musicVolume = 1;

nanotime_step_data stepper;

void windowInit(){
    // Initialize SDL

    window = nullptr;

    SDL_Init(SDL_INIT_VIDEO);

    #ifdef PORTMASTER
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    screenWidth = DM.w;
    screenHeight = DM.h;
    #endif

    window = SDL_CreateWindow(
        "Apotris PC",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screenWidth, screenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    if(SDL_NumJoysticks() > 0){
        controller = SDL_GameControllerOpen(0);
    }

    #ifdef GL
    initShaders(window);
    #endif

    refreshWindowSize();

    gSoloud.init();
    loadAudio();

    nanotime_step_init(&stepper, (uint64_t) (NANOTIME_NSEC_PER_SEC / FPS_TARGET), nanotime_now_max(),nanotime_now,nanotime_sleep);

    SDL_Surface *icon = SDL_LoadBMP("assets/favicon32.bmp");

    SDL_SetWindowIcon(window,icon);

    SDL_FreeSurface(icon);
}

bool hasSuffix(const std::string& s, const std::string& suffix){
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

void loadAudio(){
    for(int i = 0; i < SFX_COUNT; i++){
        std::string str = (std::string) SoundEffectPaths[i];

        soundEffects[i].load(str.c_str());
    }

    int counter = 0;

    std::string path = "assets";

    DIR *dir = opendir(path.c_str());
    if(!dir){
        log("couldn't open dir");
        return;
    }

    dirent *entry;
    std::vector<std::string> songPaths;
    while((entry = readdir(dir)) != nullptr){
        if(hasSuffix(entry->d_name, ".it") || hasSuffix(entry->d_name, ".mod") || hasSuffix(entry->d_name, ".xm")){
            songPaths.push_back(entry->d_name);
        }
    }

    closedir(dir);

    sort(songPaths.begin(),songPaths.end());

    for(auto str : songPaths){
        if (str.find("MENU_") != std::string::npos) {
            songList.push_back(path + "/" + str);
            songs.menu.push_back(counter++);
        } else {
            songList.push_back(path + "/" + str);
            songs.game.push_back(counter++);
        }
    }

    musicArray = new SoLoud::Openmpt[counter];

    for(int i = 0; i < counter; i++){
        SoLoud::result loaded = musicArray[i].load(songList.at(i).c_str());
        if(loaded != SoLoud::SO_NO_ERROR){
            log("couldn't load: " + songList.at(i) + " " + std::to_string(loaded));
        }
    }
}

void freeAudio(){
    delete [] musicArray;
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
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            if(key == SDLK_F11){
                if(fullscreen){
                    SDL_SetWindowFullscreen(window, 0);
                }else{
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                fullscreen = !fullscreen;
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
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
                screenWidth = event.window.data1;
                screenHeight = event.window.data2;
                refreshWindowSize();
            }
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

    #ifndef GL

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_FreeSurface(img);

    const int x = (in_width - screenWidth / windowScale);
    const int y = (in_height - screenHeight / windowScale);
    const int w = screenWidth / windowScale;
    const int h = screenHeight / windowScale;

    SDL_Rect rect = {x/2,y/2,w,h};

    SDL_RenderClear(renderer);

    SDL_Rect dest_rect;

    if(x < 0 || y < 0){
        int nx = min(x,0);
        int ny = min(y,0);
        dest_rect = {-nx/2,-ny/2,screenWidth+nx,screenHeight+ny};
    }else{
        dest_rect = {0,0,screenWidth,screenHeight};
    }

    if(render){
        SDL_RenderCopy(renderer,texture,&rect,&dest_rect);
    }

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    #else
    if(render){
        drawWithShaders(window, img, false);
    }

    SDL_FreeSurface(img);
    #endif

    // const int x = (in_width - screenWidth / windowScale);
    // const int y = (in_height - screenHeight / windowScale);
    // const int w = screenWidth / windowScale;
    // const int h = screenHeight / windowScale;

    // SDL_Rect rect = {x/2,y/2,w,h};

    // SDL_RenderClear(renderer);

    // SDL_Rect dest_rect;

    // if(x < 0 || y < 0){
    //     int nx = min(in_width * windowScale - screenWidth,0);
    //     int ny = min(y,0);
    //     dest_rect = {-nx/2,-ny/2,screenWidth+nx,screenHeight+ny};
    // }else{
    //     dest_rect = {0,0,screenWidth,screenHeight};
    // }

    // if(render){
    //     SDL_RenderCopy(renderer,texture,&rect,&dest_rect);
    // }

    // SDL_RenderPresent(renderer);

    // SDL_DestroyTexture(texture);

    // Cap FPS
    nanotime_step(&stepper);

    // //log FPS
    // uint64_t temp = frame_start;
    // frame_start = stepper.sleep_point;

    // uint64_t interval = nanotime_interval(temp, frame_start, nanotime_now_max());
    // float time = (float) interval / NANOTIME_NSEC_PER_SEC;
    // log(std::to_string(time * 3600) + " " + std::to_string(interval));
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

    rowStart = (SCREEN_HEIGHT - (screenHeight/windowScale))/2;
    rowEnd = (SCREEN_HEIGHT + (screenHeight/windowScale))/2;

    if(rowStart < 0)
        rowStart = 0;

    if(rowEnd > SCREEN_HEIGHT)
        rowEnd = SCREEN_HEIGHT;

    if(savefile != nullptr)
        setGradient(savefile->settings.backgroundGradient);

    #ifdef GL
    refreshShaderResolution(screenWidth, screenHeight,windowScale);
    #endif
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

void startSong(int song,bool loop){
    log(song);
    // if(song == currentSong && !musicArray[currentSong].hasEnded())
    if(song == currentSong && !gSoloud.getPause(currentSongHandle))
        return;

    if(currentSong != -1)
        musicArray[currentSong].stop();

    currentSongHandle = gSoloud.play(musicArray[song]);
    currentSong = song;

    gSoloud.setVolume(currentSongHandle,musicVolume);
}

void stopSong(){
    musicArray[currentSong].stop();
}

void resumeSong(){
    gSoloud.setPause(currentSongHandle,0);
}

void setMusicVolume(int volume){
    musicVolume = (float)volume / 200;

    gSoloud.setVolume(currentSongHandle,musicVolume);
}

void setMusicTempo(int tempo){
    gSoloud.setRelativePlaySpeed(currentSongHandle, tempo / 1024);
}

void toggleRendering(bool r){
    render = r;
}

void sfxRate(int n, float rate){
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    int s = gSoloud.play(soundEffects[n]);
    gSoloud.setRelativePlaySpeed(s,rate);
}

void pauseSong(){
    gSoloud.setPause(currentSongHandle,1);
}
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

    memset32_fast(temp, 0, saveSize / 4);
    memcpy32_fast(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

    if(!output){
        log("Error when trying to write save.");
        return;
    }

    output.close();
}

void quit(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    freeAudio();
    gSoloud.deinit();
    exit(0);
}

std::string stringFromKey(uint32_t key){
    return SDL_GetKeyName(key);
}

#endif
