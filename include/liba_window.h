#pragma once

#include "liba_pc.h"

#include <SFML/Window.hpp>

extern void key_poll();

extern std::list<sf::Keyboard::Key> currentKeys;
extern std::list<sf::Keyboard::Key> previousKeys;

#define KEY_A sf::Keyboard::X
#define KEY_B sf::Keyboard::Z
#define KEY_SELECT sf::Keyboard::Backspace
#define KEY_START sf::Keyboard::Enter
#define KEY_RIGHT sf::Keyboard::Right
#define KEY_LEFT sf::Keyboard::Left
#define KEY_UP sf::Keyboard::Up
#define KEY_DOWN sf::Keyboard::Down
#define KEY_R sf::Keyboard::A
#define KEY_L sf::Keyboard::S
#define KEY_FULL	0xFFFFFFFF

extern int rowStart;
extern int rowEnd;

//! Gives the keys of \a key that are currently down
INLINE u32 key_is_down(u32 key){
    if(key == KEY_FULL)
        return (!currentKeys.empty());

    auto it = currentKeys.begin();

    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));

    return (it != currentKeys.end());
}

//! Gives the keys of \a key that are pressed (down now but not before)
INLINE u32 key_hit(u32 key){
    if(key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    bool curr = (it != currentKeys.end());

    return (!prev && curr);
}

//! Gives the keys of \a key that are being released
INLINE u32 key_released(u32 key){
    if(key == KEY_FULL)
        return (!previousKeys.empty() && currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),sf::Keyboard::Key(key));
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),sf::Keyboard::Key(key));
    bool curr = (it != currentKeys.end());

    return (prev && !curr);
}

extern void refreshWindowSize();

extern void windowInit();

extern void updateWindow(u8* framebuffer);
