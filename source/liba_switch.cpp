#ifdef SWITCH

#include "liba_switch.h"
#include <list>
#include <fstream>

#include "def.h"
#include "switch.h"
#include "soloud.h"
#include "soloud_wav.h"

void loadAudio();
void freeAudio();

Songs songs;

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

// int rowStart = (288 - 288) / 2;
// int rowEnd = rowStart + 288;
int rowStart = 0;
int rowEnd = 180;

#define FPS_TARGET 60

double clock_timer = 0;

static uint64_t frame_start = SDL_GetPerformanceCounter();

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

SDL_Window* window;
SDL_Renderer * renderer;

bool render = true;

std::list<uint32_t> currentlyPressed;

SoLoud::Soloud gSoloud;

SoLoud::Wav soundEffects[SFX_COUNT];

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

    gSoloud.init();

    loadAudio();
}

void loadAudio(){
    for(int i = 0; i < SFX_COUNT; i++){
        std::string str = "romfs:/" + (std::string) SoundEffectPaths[i];

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
    // // // Update the screen
    // // SDL_UpdateRect(screen, 0, 0, 0, 0);

    // uint8_t * newBuffer = new uint8_t[640 * 480 * 4];

    // const int in_width = 320;
    // const int in_height = 240;

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
        320,
        180,
        32,
        320 * 4,
        0x0000ff,
        0x00ff00,
        0xff0000,
        0xff000000
    );

    // delete [] newBuffer;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_FreeSurface(img);

    SDL_Rect rect = {(320 - 320) / 2, (180 - 180) / 2, 320, 180};

    SDL_RenderClear(renderer);

    if(render)
        SDL_RenderCopy(renderer,texture,&rect,NULL);

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    // Cap to 60 FPS
    Uint64 end = SDL_GetPerformanceCounter();

    float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    SDL_Delay(floor(16.666f - elapsedMS));
    frame_start = SDL_GetPerformanceCounter();

}

void refreshWindowSize(){

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
