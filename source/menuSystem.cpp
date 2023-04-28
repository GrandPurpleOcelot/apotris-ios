#include "menuSystem.h"
#include "platform.hpp"
#include "logging.h"
#include "menu.h"
#include <string>
#include "scene.hpp"

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

    toggleRendering(false);
    if(previousPalette != nullptr)
        loadPalette(0, 0, previousPalette, 512);

    if(scene != nullptr)
        delete scene;

    scene = newScene;
    scene->init();

    vsync();
    toggleRendering(true);

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
    }
}

void fadeFromBlack(){
    int timer=FADE_LENGTH;

    if(previousPalette != nullptr)
        delete previousPalette;
    previousPalette = new COLOR [512];
    savePalette(previousPalette);

    color_fade_palette(0, 1, &previousPalette[1], 0, 511, timer * 8);

    while(timer >= 0){
        canDraw=1;
        vsync();
        color_fade_palette(0, 1, &previousPalette[1], 0, 511, timer * 8);
        timer--;
    }
}

void sceneSwitcher(std::string str){
    if(str == "Play"){
        changeScene(new ModeListScene(),FADE);
    }else if(str == "Settings"){
        changeScene(new SettingsScene(),FADE);
    }else if(str == "Achievements"){
        changeScene(new TestOption(),FADE);
    }else if(str == "Stats"){
    }else if(str == "Links"){
    }else if(str == "Skin Editor"){
        changeScene(new EditorScene(),FADE);
    }else if(str == "Credits"){
    }else{
        log("invalid option: " + str);
    }
}
