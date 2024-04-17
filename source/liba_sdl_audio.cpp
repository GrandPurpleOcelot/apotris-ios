#include "platform.hpp"

#if defined(PC) || defined(PORTMASTER) || defined(WEB)

#include <fstream>
#include <iostream>
#include <dirent.h>
#include <cstdio>

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_openmpt.h"

#include "def.h"

SoLoud::Soloud gSoloud;
SoLoud::Queue musicQueue;

SoLoud::Wav soundEffects[SFX_COUNT];

SoLoud::Openmpt *musicArray;

SoLoud::handle currentSongHandle;

std::vector<std::string> songList;

int currentSong = -1;
int sfxVolume = 1024;
float musicVolume = 1;

Songs songs;

bool hasSuffix(const std::string& s, const std::string& suffix){
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

void loadAudio(){
    gSoloud.init();

    for(int i = 0; i < SFX_COUNT; i++){
        std::string str = (std::string) SoundEffectPaths[i];

        soundEffects[i].load(str.c_str());
    }

    int counter = 0;

    std::string path = "assets";

    DIR *dir = opendir(path.c_str());
    if(!dir){
        log("couldn't open dir");
        return;
    }

    dirent *entry;
    std::vector<std::string> songPaths;
    while((entry = readdir(dir)) != nullptr){
        if(hasSuffix(entry->d_name, ".it") || hasSuffix(entry->d_name, ".mod") || hasSuffix(entry->d_name, ".xm")){
            songPaths.push_back(entry->d_name);
        }
    }

    closedir(dir);

    sort(songPaths.begin(),songPaths.end());

    for(auto str : songPaths){
        if (str.find("MENU_") != std::string::npos) {
            songList.push_back(path + "/" + str);
            songs.menu.push_back(counter++);
        } else {
            songList.push_back(path + "/" + str);
            songs.game.push_back(counter++);
        }
    }

    musicArray = new SoLoud::Openmpt[counter];

    for(int i = 0; i < counter; i++){
        SoLoud::result loaded = musicArray[i].load(songList.at(i).c_str());
        if(loaded != SoLoud::SO_NO_ERROR){
            log("couldn't load: " + songList.at(i) + " " + std::to_string(loaded));
        }
    }
}

void freeAudio(){
    delete [] musicArray;
    gSoloud.deinit();
}

void songEndHandler(){
    if(musicVolume == 0)
        return;

    if(currentSong != -1 && !musicQueue.isCurrentlyPlaying(musicArray[currentSong])){
        playNextSong();
    }
}

void sfx(int n){
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    gSoloud.play(soundEffects[n]);
}

void startSong(int song,bool loop){
    log(song);
    // if(song == currentSong && !musicArray[currentSong].hasEnded())
    if(song == currentSong && !gSoloud.getPause(currentSongHandle) && musicQueue.isCurrentlyPlaying(musicArray[currentSong]))
        return;

    if(currentSong != -1 && musicQueue.isCurrentlyPlaying(musicArray[currentSong])){
        // musicArray[currentSong].stop();
        musicQueue.stop();
        musicQueue.skip();
    }

    if(musicVolume > 0){
        musicQueue.setParams(44100, 2);
        currentSongHandle = gSoloud.play(musicQueue);
        musicArray[song].setLooping(loop);
        musicQueue.play(musicArray[song]);
    }

    currentSong = song;

    gSoloud.setVolume(currentSongHandle,musicVolume);

    // float speed = gSoloud.getRelativePlaySpeed(currentSongHandle);

    // gSoloud.setRelativePlaySpeed(currentSongHandle, speed * 0.5f);
}

void stopSong(){
    musicQueue.stop();
    musicQueue.skip();

    currentSong = -1;
}

void resumeSong(){
    gSoloud.setPause(currentSongHandle,0);
}

void setMusicVolume(int volume){
    if(musicVolume == 0 && volume != 0){
        musicQueue.setParams(44100, 2);
        currentSongHandle = gSoloud.play(musicQueue);
        musicQueue.play(musicArray[currentSong]);
    }

    musicVolume = volume / (512.0 / 2);

    gSoloud.setVolume(currentSongHandle,musicVolume);
}

void setMusicTempo(int tempo){
    float nt = tempo / 1024.0;

    log(std::to_string(nt));

    gSoloud.setRelativePlaySpeed(currentSongHandle, tempo / 1024.0);
}

void sfxRate(int n, float rate){
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    int s = gSoloud.play(soundEffects[n]);
    gSoloud.setRelativePlaySpeed(s,rate);
}

void pauseSong(){
    gSoloud.setPause(currentSongHandle,1);
}

#endif
