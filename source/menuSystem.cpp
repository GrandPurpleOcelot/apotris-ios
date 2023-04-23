#include "menuSystem.h"
#include "platform.hpp"
#include "logging.h"
#include "menu.h"

#define FADE_LENGTH 4

static COLOR * previousPalette = nullptr;

void fadeToBlack();
void fadeFromBlack();

void changeScene(Scene *newScene, Transitions transition){
    gradient(0);
    setLightMode();

    switch(transition){
    case INSTANT:
        break;
    case FADE:
        fadeToBlack();
        break;
    case SCANLINE:
        break;
    default:
        break;
    }

    if(scene != nullptr){
        delete scene;
    }

    scene = newScene;
    scene->init();

    switch(transition){
    case INSTANT:
        break;
    case FADE:
        fadeFromBlack();
        break;
    case SCANLINE:
        break;
    default:
        break;
    }
}

void changeScene(Scene *newScene){
    changeScene(newScene,INSTANT);
}

void fadeToBlack(){

    if(previousPalette != nullptr)
        delete previousPalette;
    previousPalette = new COLOR [512];
    savePalette(previousPalette);

    int timer=0;
    while(timer++ < FADE_LENGTH){
        vsync();

        color_fade_palette(0, 1, &previousPalette[1], 0, 511, timer * 8);
        // clr_fade_fast(temp,0x0000,pal_bg_mem,512,timer);
    }
}

void fadeFromBlack(){
    int timer=FADE_LENGTH;
    while(timer-- > 0){
        canDraw=1;
        vsync();

        color_fade_palette(0, 1, &previousPalette[1], 0, 511, timer * 8);
        // clr_fade_fast(temp,0x0000,pal_bg_mem,512,timer);
    }
}
