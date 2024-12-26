#pragma once

#include <cinttypes>

#include "liba_pc.h"
#include "audio_files.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include <map>

#define KEY_A SDLK_RETURN
#define KEY_B SDLK_BACKSPACE
#define KEY_L SDLK_1
#define KEY_R SDLK_2
#define KEY_UP SDLK_w
#define KEY_DOWN SDLK_s
#define KEY_LEFT SDLK_a
#define KEY_RIGHT SDLK_d
#define KEY_SELECT SDLK_3
#define KEY_START SDLK_ESCAPE
#define KEY_FULL 0xffffffff


extern void updateWindow(uint8_t*);
extern void refreshWindowSize();
extern bool closed();

extern void windowInit();

extern void key_poll();

extern void setFullscreen(bool);

extern uint32_t key_is_down(uint32_t);
extern uint32_t key_hit(uint32_t);
extern uint32_t key_released(uint32_t);
extern uint32_t key_first();

extern int rowStart;
extern int rowEnd;

std::string stringFromKey(uint32_t key);
