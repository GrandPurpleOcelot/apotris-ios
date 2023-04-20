#include "menuSystem.h"
#include "platform.hpp"
#include "logging.h"
#include "menu.h"

void changeScene(Scene *newScene){
    if(scene != nullptr){
        delete scene;
    }

    scene = newScene;
    scene->init();
}
