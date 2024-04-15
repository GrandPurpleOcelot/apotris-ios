#pragma once

extern void sfx(int);
extern void startSong(int,bool);
extern void stopSong();
extern void resumeSong();
extern void setMusicVolume(int);
extern void setMusicTempo(int);
extern void toggleRendering(bool);
extern void sfxRate(int,float);
extern void pauseSong();

extern void loadAudio();
extern void freeAudio();
extern void songEndHandler();
