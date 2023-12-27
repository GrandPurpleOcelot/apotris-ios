#include "sceneAudio.hpp"
#include "logging.h"

void SongListElement::action(){
    sfx(SFX_MENUCONFIRM);
    changeScene(new SongListScene(), Transitions::FADE);
}

void SongElement::select(){
    playSong(menu, index);
}

void SongListScene::init(){
    OptionListScene::init();

    auto it = elementList.begin();

    do{
        std::advance(it,1);
    }while((*it)->getCurrentOption() == "\n");

    SongElement * song = (SongElement *) *it;

    song->select();
}

bool SongListScene::control(){
    int previous = selection;
    OptionListScene::control();

    if(selection != previous){
        auto it = elementList.begin();
        std::advance(it,selection);

        SongElement * song = (SongElement *) *it;

        song->select();
    }

    return false;
}
