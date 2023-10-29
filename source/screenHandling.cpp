#include "sceneHandling.hpp"

bool HandlingOptionScene::control(){
    OptionListScene::control();

    MenuKeys k = savefile->settings.menuKeys;

    if(key_hit(k.special2)){
        handlingTest = true;

        BlockEngine::Options options;
        options.tuning = getTuning();

        startGame(options,qran());

        changeScene(new GameScene(), Transitions::FADE);
    }

    return false;
}
