#ifdef MM

#include "liba_mm.h"
#include <list>
#include <fstream>

#include "SDL/SDL.h"
#include "def.h"

SDL_Surface * screen;

std::list<SDL_keysym> currentKeys;
std::list<SDL_keysym> previousKeys;

// int rowStart = (512 - 240) / 2;
// int rowEnd = rowStart + 240;
int rowStart = 0;
int rowEnd = 160;

#define FPS_TARGET 60

double clock_timer = 0;

void windowInit(){
    SDL_Init(SDL_INIT_VIDEO);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return;
    }

    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("Could not create screen! SDL Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }
}

void key_poll(){
    previousKeys = currentKeys;
    currentKeys.clear();

    SDL_Event event;

    while( SDL_PollEvent( &event ) ){
        switch( event.type ){
        case SDL_KEYDOWN:
            currentKeys.push_back(event.key.keysym);
            break;
        case SDL_QUIT:
            std::cout << "quitting!\n";
            SDL_Quit();
            break;
        default:
            break;
        }
    }
}

uint32_t key_is_down(uint32_t){
    // if(key == KEY_FULL)
    //     return (!currentKeys.empty());

    // auto it = currentKeys.begin();

    // it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));

    // return (it != currentKeys.end());

    return 0;
}

uint32_t key_hit(uint32_t){
    // if(key == KEY_FULL)
    //     return (previousKeys.empty() && !currentKeys.empty());

    // auto it = previousKeys.begin();
    // it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    // bool prev = (it != previousKeys.end());

    // it = currentKeys.begin();
    // it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    // bool curr = (it != currentKeys.end());

    // return (!prev && curr);

    return 0;
}

uint32_t key_released(uint32_t){
    // if(key == KEY_FULL)
    //     return (!previousKeys.empty() && currentKeys.empty());

    // auto it = previousKeys.begin();
    // it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    // bool prev = (it != previousKeys.end());

    // it = currentKeys.begin();
    // it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    // bool curr = (it != currentKeys.end());

    // return (prev && !curr);

    return 0;
}

void updateWindow(uint8_t* framebuffer){

    // // Fill screen with specified background color
    // SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 20, 0, 0));

    // // Update the screen
    // SDL_UpdateRect(screen, 0, 0, 0, 0);

    // Load image
    // SDL_Surface* img = IMG_Load(image_path);
    // if (img == NULL) {
    //     printf("Unable to load image %s! SDL_image Error: %s\n", image_path, IMG_GetError());
    //     SDL_Quit();
    //     return 1;
    // }

    uint8_t * newBuffer = new uint8_t[640 * 480 * 4];

    // memset32(newBuffer,0x1234,640*480);

    uint32_t* out_ptr = (uint32_t *) newBuffer;
    uint32_t* in_ptr = (uint32_t *) framebuffer;
    int in_pitch = 512;
    int out_pitch = 640;

    in_ptr += (((512 - 160) / 2) + 160) * in_pitch;
    in_ptr -= (512 - 214) / 2;

    for(int y = 0; y < 160; y++){
        uint32_t * line_ptr = in_ptr;

        for(int x = 0; x < 214; x++){
            uint32_t * out_line_ptr = out_ptr;
            uint32_t color = *line_ptr;

            *out_line_ptr = color;

            if(x != 213){
                *(out_line_ptr + 1) = color;
                *(out_line_ptr + 2) = color;

                out_line_ptr += out_pitch;
                *out_line_ptr = color;
                *(out_line_ptr + 1) = color;
                *(out_line_ptr + 2) = color;

                out_line_ptr += out_pitch;
                *out_line_ptr = color;
                *(out_line_ptr + 1) = color;
                *(out_line_ptr + 2) = color;

                out_ptr += 3;
            }else{
                out_line_ptr += out_pitch;
                *out_line_ptr = color;

                out_line_ptr += out_pitch;
                *out_line_ptr = color;

                out_ptr += 1;
            }

            line_ptr--;
        }

        in_ptr -= in_pitch;
        out_ptr += out_pitch * 2;
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
        0
    );

    // Center image on screen
    SDL_Rect dstrect;
    dstrect.x = (screen->w - img->w) / 2;
    dstrect.y = (screen->h - img->h) / 2;

    // Draw image on screen
    SDL_BlitSurface(img, NULL, screen, &dstrect);
    SDL_Flip(screen);

    // Wait 5 seconds then quit
    // SDL_Delay(5000);


    // Free surfaces
    SDL_FreeSurface(img);
    delete [] newBuffer;

    // SDL_Quit();
    //

}

void refreshWindowSize(){

}

bool closed(){
    double newclock = SDL_GetTicks();

    double deltaticks = 1000 / FPS_TARGET - (newclock - clock_timer);

    if((int)deltaticks > 0)
        SDL_Delay(deltaticks);

    if(deltaticks < -30){
        clock_timer = newclock - 30;

    }else{
        clock_timer = newclock + deltaticks;
    }

    return true;
}

void sfx(int){

}

void startSong(int,bool){

}

void stopSong(){};

void resumeSong(){};

void setMusicVolume(int){};

void setMusicTempo(int){};

void toggleRendering(bool){};

void sfxRate(int,float){};

void pauseSong(){};
void initRumble(){};

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

#endif
