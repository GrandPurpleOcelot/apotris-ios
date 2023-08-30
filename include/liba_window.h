#pragma once

#include "liba_pc.h"

#include <SFML/Window.hpp>
#include "audio_files.h"

extern void key_poll();

extern std::list<int> currentKeys;
extern std::list<int> previousKeys;

extern std::vector<std::string> keyToString;

#define KEY_A sf::Keyboard::Enter
#define KEY_B sf::Keyboard::Backspace
#define KEY_SELECT sf::Keyboard::Num1
#define KEY_START sf::Keyboard::Escape
#define KEY_RIGHT sf::Keyboard::D | (sf::Keyboard::Right << 8)
#define KEY_LEFT sf::Keyboard::A | (sf::Keyboard::Left << 8)
#define KEY_UP sf::Keyboard::W | (sf::Keyboard::Up << 8)
#define KEY_DOWN sf::Keyboard::S | (sf::Keyboard::Down << 8)
#define KEY_L sf::Keyboard::Num3
#define KEY_R sf::Keyboard::Num4
#define KEY_FULL	0xFFFFFFFF

extern int rowStart;
extern int rowEnd;

//! Gives the keys of \a key that are currently down
INLINE u32 key_is_down(u32 key){
    if(key == KEY_FULL)
        return (!currentKeys.empty());

    int high = key >> 8;
    bool res = false;

    if(high){
        res = key_is_down(high);
        key &= 0xff;
    }

    auto it = currentKeys.begin();

    it = std::find(currentKeys.begin(),currentKeys.end(),key + 1);

    return (it != currentKeys.end()) || res;
}

//! Gives the keys of \a key that are pressed (down now but not before)
INLINE u32 key_hit(u32 key){
    if(key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty());

    int high = key >> 8;
    bool res = false;

    if(high){
        res = key_hit(high);
        key &= 0xff;
    }

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),key + 1);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),key + 1);
    bool curr = (it != currentKeys.end());

    return (!prev && curr) || res;
}

//! Gives the keys of \a key that are being released
INLINE u32 key_released(u32 key){
    if(key == KEY_FULL)
        return (!previousKeys.empty() && currentKeys.empty());

    int high = key >> 8;
    bool res = false;

    if(high){
        res = key_released(high);
        key &= 0xff;
    }

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(),previousKeys.end(),key + 1);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(),currentKeys.end(),key + 1);
    bool curr = (it != currentKeys.end());

    return (prev && !curr) || res;
}

extern void refreshWindowSize();

extern void windowInit();

extern void updateWindow(u8* framebuffer);
