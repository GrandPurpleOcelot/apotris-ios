#include "sceneAudio.hpp"
#include "logging.h"

bool SongListElement::action(){
    sfx(SFX_MENUCONFIRM);
    changeScene(new SongListScene(), Transitions::FADE);
    return true;
}

void SongElement::select() const{
    playSong(menu, index);
}

void SongListScene::init(){
    OptionListScene::init();

    auto it = elementList.begin();

    do{
        std::advance(it,1);
    }while((*it)->getCurrentOption() == "\n");

    auto * song = (SongElement *) *it;

    song->select();
}

bool SongListScene::control(){
    int previous = selection;
    if(OptionListScene::control())
        return true;

    if(selection != previous){
        auto it = elementList.begin();
        std::advance(it,selection);

        auto * song = (SongElement *) *it;

        song->select();
    }

    return false;
}
