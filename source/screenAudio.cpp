#include "logging.h"
#include "sceneAudio.hpp"

bool TrackListElement::action() {
    sfx(SFX_MENUCONFIRM);
    changeScene(new TrackListScene(), Transitions::FADE);
    return true;
}

void SongElement::select() {
    if (index < 0)
        return;
    playSong(menu, index);
}

void TrackListScene::init() {
    OptionListScene::init();

    auto it = elementList.begin();

    do {
        std::advance(it, 1);
    } while ((*it)->getCurrentOption() == "\n");

    SongElement* song = (SongElement*)*it;

    song->select();
}

bool TrackListScene::control() {
    int previous = selection;
    if (OptionListScene::control())
        return true;

    if (selection != previous) {
        auto it = elementList.begin();
        std::advance(it, selection);

        SongElement* song = (SongElement*)*it;

        song->select();
    }

    return false;
}
