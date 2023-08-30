#pragma once

#include <cinttypes>

#include "liba_pc.h"
#include "audio_files.h"

#include "SDL2/SDL.h"
#include <SDL2/SDL_mixer.h>

#define KEY_A SDLK_x
#define KEY_B SDLK_z
#define KEY_L SDLK_a
#define KEY_R SDLK_s
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_SELECT SDLK_BACKSPACE
#define KEY_START SDLK_RETURN
#define KEY_FULL 0xffffffff

extern void updateWindow(uint8_t*);
extern void refreshWindowSize();
extern bool closed();

extern void sfx(int);
extern void startSong(int,bool);
extern void stopSong();
extern void resumeSong();
extern void setMusicVolume(int);
extern void setMusicTempo(int);
extern void toggleRendering(bool);
extern void sfxRate(int,float);

extern void pauseSong();

extern void windowInit();

extern void key_poll();

extern uint32_t key_is_down(uint32_t);
extern uint32_t key_hit(uint32_t);
extern uint32_t key_released(uint32_t);
extern uint32_t key_first();

extern int rowStart;
extern int rowEnd;
