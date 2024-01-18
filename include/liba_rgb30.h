#pragma once

#include <cinttypes>

#include "liba_pc.h"
#include "audio_files.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

#include <map>

#define KEY_A SDL_CONTROLLER_BUTTON_A
#define KEY_B SDL_CONTROLLER_BUTTON_B
#define KEY_X SDL_CONTROLLER_BUTTON_X
#define KEY_Y SDL_CONTROLLER_BUTTON_Y
#define KEY_START SDL_CONTROLLER_BUTTON_START  // Assuming PLUS is the START button
#define KEY_SELECT SDL_CONTROLLER_BUTTON_BACK  // Assuming MINUS is the BACK button
#define KEY_LEFT SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define KEY_UP SDL_CONTROLLER_BUTTON_DPAD_UP
#define KEY_RIGHT SDL_CONTROLLER_BUTTON_DPAD_RIGHT
#define KEY_DOWN SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define KEY_L SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define KEY_R SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
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

extern std::map<int,std::string> keyToString;
