#include "def.h"
#include "logging.h"

int currentMenu = 0;
int currentlyPlayingSong = 0;

void playSong(int menuId, int songId) {
    int song = 0;

    std::list<int> *songList;

    if (menuId == 0) {
        songList = &songs.menu;
    } else if (menuId == 1) {
        songList = &songs.game;
    }

    if(songId > (int) songList->size())
        return;

    auto it = songList->begin();
    std::advance(it,songId);

    song = *it;

    currentMenu = menuId;
    currentlyPlayingSong = songId;

    startSong(song,(!savefile->settings.cycleSongs));
    setMusicVolume(512 * ((float)savefile->settings.volume / 10));
}

void playSongRandom(int menuId) {

    int songId = -1;
    int max = 0;

    if (menuId == 0) {
        max = songs.menu.size();
    } else if (menuId == 1) {
        max = songs.game.size();
    }

    if (max <= 0)
        return;

    int index = qran() % max;

    songId = index;

    if (songId == -1)
        return;

    playSong(menuId, songId);
}

void playNextSong(){
    int songId = -1;
    int max = 0;

    int menuId = currentMenu;

    if (menuId == 0) {
        max = songs.menu.size();
    } else if (menuId == 1) {
        max = songs.game.size();
    }

    if (max <= 0)
        return;

    int index = currentlyPlayingSong+1;
    if(index >= max)
        index = 0;

    songId = index;

    if (songId == -1)
        return;

    playSong(menuId, songId);
}
