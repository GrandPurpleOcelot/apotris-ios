#include "platform.hpp"
#include "menuSystem.h"
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
    case Transitions::INSTANT:
        break;
    case Transitions::FADE:
        fadeToBlack();
        break;
    case Transitions::SCANLINE:
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
    case Transitions::INSTANT:
        break;
    case Transitions::FADE:
        fadeFromBlack();
        break;
    case Transitions::SCANLINE:
        break;
        default:
        break;
    }
}

void changeScene(Scene *newScene){
    changeScene(newScene,Transitions::INSTANT);
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
        changeScene(new ModeListScene(),Transitions::FADE);
    }else if(str == "Settings"){
        changeScene(new SettingsScene(),Transitions::FADE);
    }else if(str == "Achievements"){

    }else if(str == "Stats"){
        changeScene(new StatScene(),Transitions::FADE);
    }else if(str == "Links"){

    }else if(str == "Skin Editor"){
        changeScene(new EditorScene(),Transitions::FADE);
    }else if(str == "Credits"){

    }else if(str == "Graphics"){
        changeScene(new newGraphicsScene(),Transitions::FADE);
    }else if(str == "Audio"){
        changeScene(new AudioOptionScene(),Transitions::FADE);
    }else if(str == "Controls"){
        changeScene(new ControlOptionScene(),Transitions::FADE);
    }else if(str == "Handling"){
        changeScene(new HandlingOptionScene(),Transitions::FADE);
    }else if(str == "Marathon"){
        changeScene(new MarathonScene(),Transitions::FADE);
    }else if(str == "Sprint"){
        changeScene(new SprintScene(),Transitions::FADE);
    }else if(str == "Dig"){
        changeScene(new DigScene(),Transitions::FADE);
    }else if(str == "Ultra"){
        changeScene(new UltraScene(),Transitions::FADE);
    }else if(str == "Blitz"){
        changeScene(new BlitzScene(),Transitions::FADE);
    }else if(str == "Combo"){
        changeScene(new ComboScene(),Transitions::FADE);
    }else if(str == "Survival"){
        changeScene(new SurvivalScene(),Transitions::FADE);
    }else if(str == "Classic"){
        changeScene(new ClassicScene(),Transitions::FADE);
    }else{
        log("invalid option: " + str);
    }
}
