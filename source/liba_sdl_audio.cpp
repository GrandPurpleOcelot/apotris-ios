#include "platform.hpp"

#if defined(PC) || defined(PORTMASTER) || defined(WEB) || defined(SWITCH) || defined(ANDROID)

#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "soloud.h"
#include "soloud_openmpt.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "def.h"

SoLoud::Soloud gSoloud;
SoLoud::Queue musicQueue;

SoLoud::Wav soundEffects[SFX_COUNT];

std::vector<SoLoud::AudioSource*> musicArray;

SoLoud::handle currentSongHandle;

std::vector<std::string> songList;
std::vector<std::string> songNameList;

int currentSong = -1;
int sfxVolume = 1024;
float musicVolume = 1;
bool looping = false;

Songs songs;

bool hasSuffix(const std::string& s, const std::string& suffix) {
    return (s.size() >= suffix.size()) &&
           equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

void loadAudio(std::string pathPrefix) {
    gSoloud.init();

    for (int i = 0; i < SFX_COUNT; i++) {
        std::string str = pathPrefix + (std::string)SoundEffectPaths[i];

#ifdef ANDROID
        size_t size;
        void* data = SDL_LoadFile(str.c_str(), &size);
        if (data) {
             soundEffects[i].loadMem((unsigned char*)data, size, true, false);
             SDL_free(data);
        }
#else
        soundEffects[i].load(str.c_str());
#endif
    }

    int counter = 0;

    std::string path = pathPrefix + "assets";
    std::vector<std::string> songPaths;

#ifdef ANDROID
    size_t listSize;
    void* listData = SDL_LoadFile((path + "/file_list.txt").c_str(), &listSize);
    if (listData) {
        std::string listStr((char*)listData, listSize);
        SDL_free(listData);

        std::stringstream ss(listStr);
        std::string line;
        while (std::getline(ss, line)) {
            // Trim whitespace
            line.erase(line.find_last_not_of(" \n\r\t") + 1);
            if (hasSuffix(line, ".it") ||
                hasSuffix(line, ".mod") ||
                hasSuffix(line, ".s3m") ||
                hasSuffix(line, ".S3M") ||
                hasSuffix(line, ".xm")) {
                songPaths.push_back(line);
            }
        }
    }
#else
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        log("couldn't open dir");
        return;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (hasSuffix(entry->d_name, ".it") ||
            hasSuffix(entry->d_name, ".mod") ||
            hasSuffix(entry->d_name, ".s3m") ||
            hasSuffix(entry->d_name, ".S3M") ||
            hasSuffix(entry->d_name, ".mp3") ||
            hasSuffix(entry->d_name, ".ogg") ||
            hasSuffix(entry->d_name, ".flac") ||
            hasSuffix(entry->d_name, ".wav") ||
            hasSuffix(entry->d_name, ".xm")) {

            bool found = false;
            if (hasSuffix(entry->d_name, ".wav")) {
                for (int i = 0; i < SFX_COUNT; i++) {
                    std::string str = (std::string)SoundEffectPaths[i];

                    if (hasSuffix(str, entry->d_name)) {
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                songPaths.push_back(entry->d_name);
            }
        }
    }

    closedir(dir);

    sort(songPaths.begin(), songPaths.end());
#endif

    for (auto str : songPaths) {
        songList.push_back(path + "/" + str);
        songNameList.push_back(str);

        if (str.find("MENU_") != std::string::npos) {
            songs.menu.push_back(counter++);
        } else {
            songs.game.push_back(counter++);
        }
    }

    for (int i = 0; i < counter; i++) {
        SoLoud::result loaded;
        std::string songPath = songList.at(i);

#ifdef ANDROID
        size_t size;
        void* data = SDL_LoadFile(songPath.c_str(), &size);
        if (data) {
             if (hasSuffix(songPath, ".mp3") || hasSuffix(songPath, ".ogg") ||
                hasSuffix(songPath, ".flac") || hasSuffix(songPath, ".wav")) {
                SoLoud::WavStream* wavstream = new SoLoud::WavStream;
                loaded = wavstream->loadMem((unsigned char*)data, size, true, false);
                musicArray.push_back((SoLoud::AudioSource*)wavstream);
            } else {
                SoLoud::Openmpt* openmpt = new SoLoud::Openmpt;
                loaded = openmpt->loadMem((unsigned char*)data, size, true, false);
                musicArray.push_back((SoLoud::AudioSource*)openmpt);
            }
            SDL_free(data);
        } else {
            loaded = SoLoud::FILE_NOT_FOUND;
        }
#else
        if (hasSuffix(songPath, ".mp3") || hasSuffix(songPath, ".ogg") ||
            hasSuffix(songPath, ".flac") || hasSuffix(songPath, ".wav")) {
            SoLoud::WavStream* wavstream = new SoLoud::WavStream;
            loaded = wavstream->load(songPath.c_str());
            musicArray.push_back((SoLoud::AudioSource*)wavstream);
        } else {
            SoLoud::Openmpt* openmpt = new SoLoud::Openmpt;
            loaded = openmpt->load(songPath.c_str());
            musicArray.push_back((SoLoud::AudioSource*)openmpt);
        }
#endif

        if (loaded != SoLoud::SO_NO_ERROR) {
            log("couldn't load: " + songList.at(i) + " " +
                std::to_string(loaded));
        }
    }
}

void freeAudio() {
    for (auto source : musicArray) {
        delete source;
    }

    gSoloud.deinit();
}

void songEndHandler() {
    if (musicVolume == 0)
        return;

    if (currentSong != -1 &&
        !musicQueue.isCurrentlyPlaying(*musicArray.at(currentSong))) {
        if (savefile->settings.cycleSongs == 1) { // CYCLE
            playNextSong();
        } else if (savefile->settings.cycleSongs == 2) { // SHUFFLE
            playSongRandom(currentMenu);
        }
    }
}

void sfx(int n) {
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    gSoloud.play(soundEffects[n]);
}

void startSong(int song, bool loop) {
    if (song == currentSong && !gSoloud.getPause(currentSongHandle) &&
        (currentSong != -1 &&
         musicQueue.isCurrentlyPlaying(*musicArray.at(currentSong)))) {
        return;
    }

    if (currentSong != -1 &&
        musicQueue.isCurrentlyPlaying(*musicArray.at(currentSong))) {
        musicQueue.stop();
        musicQueue.skip();
    }

    if (musicVolume > 0) {
        musicQueue.setParams(44100, 2);
        currentSongHandle = gSoloud.play(musicQueue);
        musicArray.at(song)->setLooping(loop);
        musicQueue.play(*musicArray.at(song));
    }

    currentSong = song;
    looping = loop;

    gSoloud.setVolume(currentSongHandle, musicVolume);
}

void stopSong() {
    if (currentSong == -1)
        return;

    musicQueue.stop();

    musicQueue.skip();

    currentSong = -1;
}

void resumeSong() { gSoloud.setPause(currentSongHandle, 0); }

void setMusicVolume(int volume) {
    musicVolume = volume / (512.0 / 2);

    gSoloud.setVolume(currentSongHandle, musicVolume);
}

void setMusicTempo(int tempo) {
    float nt = tempo / 1024.0;

    log(std::to_string(nt));

    gSoloud.setRelativePlaySpeed(currentSongHandle, tempo / 1024.0);
}

void sfxRate(int n, float rate) {
    soundEffects[n].setVolume((float)savefile->settings.sfxVolume / 10);
    int s = gSoloud.play(soundEffects[n]);
    gSoloud.setRelativePlaySpeed(s, rate);
}

void pauseSong() { gSoloud.setPause(currentSongHandle, 1); }

std::string getSongName(int song) { return songNameList[song]; }

#endif
