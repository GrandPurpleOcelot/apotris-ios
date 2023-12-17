#include "sceneAudio.hpp"

void SongListElement::action(){
    sfx(SFX_MENUCONFIRM);
    changeScene(new SongListScene(), Transitions::FADE);
}
