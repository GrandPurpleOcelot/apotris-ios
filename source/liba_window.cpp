#ifdef PC

#include "liba_window.h"
#include "liba_pc.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "sfMod.h"

#include "def.h"
#include <fstream>
#include <cstdio>
#include <vector>
#include <dirent.h>
#include <algorithm>

#define ENABLE_SHADERS 0

float windowScale = 3;
int windowWidth = 400;
int windowHeight = 240;

int rowStart = (SCREEN_HEIGHT - windowHeight)/2;
int rowEnd = (SCREEN_HEIGHT + windowHeight)/2;

bool render = true;

void resize(int width, int height);
std::list<int> currentKeys;
std::list<int> previousKeys;

std::list<sf::Sound*> soundList;

sf::SoundBuffer sfxBuffers[SFX_COUNT];
sfmod::Mod *musicArray;

sf::Shader shader;

int currentSong = -1;
int sfxVolume = 1024;
int musicVolume = 0;

const int sfxVolumeMultiplier = 4;

Songs songs;

std::vector<std::string> songList;

bool hasSuffix(const std::string& s, const std::string& suffix){
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

void loadAudio(){
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
        if (str.find("GAME_") != std::string::npos) {
            songList.push_back(path + "/" + str);
            songs.game.push_back(counter++);
        } else {
            songList.push_back(path + "/" + str);
            songs.menu.push_back(counter++);
        }
    }

    for(int i = 0; i < SFX_COUNT; i++){
        if(!sfxBuffers[i].loadFromFile(SoundEffectPaths[i])){
            std::string str = "Failed to load ";
            str += SoundEffectPaths[i];
            log(str);
        }
    }

    musicArray = new sfmod::Mod [counter];

    for(int i = 0; i < counter; i++){
        if(!musicArray[i].loadFromFile(songList.at(i))){
            std::string str = "Failed to load ";
            str += songList.at(i);
            log(str);
        }
    }
}

void freeAudio(){
    // for(int i = 0; i < SFX_COUNT; i++){
    //     // delete sfxBuffers[i];
    // }
    for(auto sound : soundList){
        delete sound;
    }

    for(int i = 0; i < MUSIC_COUNT; i++){
        musicArray[i].unload();
    }
}

sf::RenderWindow window(sf::VideoMode(windowWidth * windowScale, windowHeight * windowScale), "Apotris PC");
sf::Texture frameTexture;

bool isFullscreen = false;

void windowInit(){
    window.setFramerateLimit(60);
    frameTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    sf::Image icon;
    icon.loadFromFile("assets/favicon32.bmp");

    window.setIcon(32, 32, icon.getPixelsPtr());

    loadAudio();

    // if(!sf::Shader::isAvailable()){
    //     log("Shaders not available on system.");
    // }else if(ENABLE_SHADERS){
    //     if(!shader.loadFromFile("assets/bloom.glsl", sf::Shader::Fragment)){
    //         log("Couldn't load Bloom shader.");
    //     }else{
    //         log("Loaded Bloom Shader.");
    //         shader.setUniform("pixelWidth",1.f / window.getSize().x);
    //         shader.setUniform("pixelHeight",1.f / window.getSize().y);
    //     }

    // }

    if(sf::Joystick::isConnected(0)){
        log("controller connected");

        int buttonCount = sf::Joystick::getButtonCount(0);

        log("button count: " + std::to_string(buttonCount));

        int axii = 0;

        for(int i = 0; i < 8; i++)
            axii += sf::Joystick::hasAxis(0, sf::Joystick::Axis(i));

        log("axis count: " + std::to_string(axii));

        log(sf::Joystick::getIdentification(0).name);

        return;
    }else{
        log("no controller connected");
    }

    resize(windowWidth * windowScale, windowHeight * windowScale);

}

void updateWindow(u8 *framebuffer){
    frameTexture.update((sf::Uint8*)framebuffer);
    sf::Sprite sprite;
    sprite.setTexture(frameTexture);
    sprite.setScale(windowScale,windowScale);
    sprite.setPosition(0,0);

    if(ENABLE_SHADERS && render){
        window.draw(sprite, &shader);

        sf::Color c = sf::Color::Black;

        c.a = 60;

        for(int i = 0; i < SCREEN_HEIGHT; i++){
            sf::RectangleShape rectangle(sf::Vector2f(SCREEN_WIDTH * windowScale,windowScale/2.0));

            rectangle.setPosition(sf::Vector2f(0,i * windowScale + windowScale / 2.0));

            rectangle.setFillColor(c);

            window.draw(rectangle);

            // sf::Vertex line[2];

            // line[0].position = sf::Vector2f(0,i * windowScale + (windowScale - 1));
            // line[1].position = sf::Vector2f(SCREEN_WIDTH * windowScale,i * windowScale + (windowScale - 1));

            // line[0].color = c;
            // line[1].color = c;

            // window.draw(line,2,sf::Lines);
        }

        // for(int i = 0; i < SCREEN_WIDTH; i++){
        //     sf::Vertex line[2];

        //     line[0].position = sf::Vector2f(i * windowScale + (windowScale),0);
        //     line[1].position = sf::Vector2f(i * windowScale + (windowScale),SCREEN_HEIGHT * windowScale);

        //     line[0].color = c;
        //     line[1].color = c;

        //     window.draw(line,2,sf::Lines);
        // }

    }else{
        if(render)
            window.draw(sprite);
    }

    window.display();

    // TLN_DrawFrame(0);
    sf::Event event;
    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed){
            quit();
        }else if(event.type == sf::Event::Resized){
            resize(event.size.width, event.size.height);

        }else if(event.type == sf::Event::KeyPressed){
            if(event.key.code == sf::Keyboard::F11){
                isFullscreen = !isFullscreen;

                if(isFullscreen){
                    sf::VideoMode mode = sf::VideoMode::getDesktopMode();

                    window.create(mode, "Apotris PC",sf::Style::Fullscreen);

                    resize(mode.width, mode.height);
                }else{
                    window.create(sf::VideoMode(windowWidth * windowScale, windowHeight * windowScale), "Apotris PC");

                    resize(windowWidth * windowScale, windowHeight * windowScale);
                }
                window.setFramerateLimit(60);
            }
        }
    }

    #ifndef NO_DIAGNOSE
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)){
        window.setFramerateLimit(2000);

        if(currentSong >= 0)
            musicArray[currentSong].setPitch(3);
    }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)){
        window.setFramerateLimit(5);
    }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)){
        window.setFramerateLimit(1);
    }else{
        window.setFramerateLimit(60);

        if(currentSong >= 0)
            musicArray[currentSong].setPitch(1);
    }

    #endif

    auto it = soundList.begin();

    while(it != soundList.end()){
        if((*it)->getStatus() == sf::Sound::Stopped){
            delete *it;
            soundList.erase(it++);
        }else{
            it++;
        }
    }

    if(currentSong >= 0 && musicArray[currentSong].getStatus() == sfmod::Mod::Stopped && musicVolume / 2 != 0){
        if(savefile->settings.cycleSongs){
            log("next");
            playNextSong();
        }else{
            log("loop");
            musicArray[currentSong].stop();
            musicArray[currentSong].play();
        }
    }

    // window.draw();

    // Uint64 end = SDL_GetPerformanceCounter();

    // float elapsedMS = (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    // // Cap to 60 FPS
    // SDL_Delay(floor(16.666f - elapsedMS));

}

void setMusicTempo(int tempo){

}

void setMusicVolume(int volume){
    musicVolume = volume;

    if(currentSong != -1){
        int f = volume / 4;

        if(f){
            musicArray[currentSong].setMasterVolume(f);
            if(musicArray[currentSong].getStatus() != sfmod::Mod::Playing)
                musicArray[currentSong].play();
        }else{
            musicArray[currentSong].pause();
        }
    }
}

void startSong(int song, bool loop){
    log("song:");
    log(song);
    if(song == currentSong && musicArray[currentSong].getStatus() == sfmod::Mod::Playing)
        return;

    if(currentSong != -1)
        musicArray[currentSong].stop();

    log(songList.at(song));

    musicArray[song].play();
    currentSong = song;
}

void sfx(int n){
    sf::Sound * sound = new sf::Sound();
    sound->setBuffer(sfxBuffers[n]);
    sound->play();
    sound->setVolume(savefile->settings.sfxVolume * sfxVolumeMultiplier);

    soundList.push_back(sound);
}

void sfxRate(int n, float rate){
    sf::Sound * sound = new sf::Sound();
    sound->setBuffer(sfxBuffers[n]);
    sound->play();
    sound->setVolume(savefile->settings.sfxVolume * sfxVolumeMultiplier);

    sound->setPitch(rate);

    soundList.push_back(sound);

    // sfx(sound);
}

void key_poll(){
    previousKeys = currentKeys;
    currentKeys.clear();

    if(!window.hasFocus()){
        return;
    }

    // if(sf::Joystick::isConnected(0)){

    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 1)) * KEY_A;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 0)) * KEY_B;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 5)) * KEY_R;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 4)) * KEY_L;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 7)) * KEY_START;
    //     __key_curr |= (sf::Joystick::isButtonPressed(0, 6)) * KEY_SELECT;

    //     float pos = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis(7));
    //     if(pos < 0)
    //         __key_curr |= (1) * KEY_UP;
    //     else if (pos > 0)
    //         __key_curr |= (1) * KEY_DOWN;

    //     pos = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis(6));
    //     if(pos < 0)
    //         __key_curr |= (1) * KEY_LEFT;
    //     else if (pos > 0)
    //         __key_curr |= (1) * KEY_RIGHT;

    // }

    for(int i = 0; i < sf::Keyboard::KeyCount; i++){
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key(i)) && sf::Keyboard::Key(i) != sf::Keyboard::Tab)
            currentKeys.push_back(i + 1);
    }
}

bool closed(){
    bool result = window.isOpen();

    return result;
}

void toggleRendering(bool state){
    render = state;
}

void resize(int width, int height){
    if(savefile != nullptr && savefile->settings.integerScale != 1){

        float zoom = ((float) max(savefile->settings.zoom,10) / 20);

        windowScale = ((float) height / 160) * zoom;
    }else{
        windowScale = (int) windowScale;

        while(height / windowScale > 160 * 2){
            windowScale++;
        }
        while(windowScale > 1 && height / windowScale < 160){
            windowScale--;
        }
    }

    sf::FloatRect visibleArea(sf::Vector2f(0,0), sf::Vector2f(width, height));
    sf::View view(visibleArea);
    view.setCenter(256 * windowScale,256 * windowScale);
    window.setView(view);

    rowStart = (512 - (height/windowScale))/2;
    rowEnd = (512 + (height/windowScale))/2;

    if(savefile != nullptr)
        setGradient(savefile->settings.backgroundGradient);
}

void refreshWindowSize(){
    resize(window.getSize().x, window.getSize().y);
}

void stopSong(){
    if(currentSong == -1)
        return;

    musicArray[currentSong].stop();

    currentSong = -1;
};

void pauseSong(){
    if(currentSong == -1)
        return;

    musicArray[currentSong].pause();
};

void resumeSong(){
    if(currentSong == -1 || savefile->settings.volume == 0)
        return;

    musicArray[currentSong].play();
};

void loadSavefile(){
    std::ifstream input("Apotris.sav",std::ios::binary | std::ios::in);

    if(savefile == nullptr)
        savefile = new Save();

    char * src = (char * ) savefile;

    input.read(src,sizeof(Save));

    if(!input){
        log("Error when trying to load save.");
        return;
    }

    input.close();
}

void saveSavefile(){
    std::ofstream output("Apotris.sav",std::ios::binary | std::ios::out);

    char * dst = (char * ) savefile;

    const int saveSize = 1 << 15;

    char temp[saveSize];

    memset32(temp, 0, saveSize / 4);
    memcpy32(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

    if(!output){
        log("Error when trying to write save.");
        return;
    }

    output.close();
}

std::vector<std::string> keyToString = {
    "A",        //!< The A key
    "B",            //!< The B key
    "C",            //!< The C key
    "D",            //!< The D key
    "E",            //!< The E key
    "F",            //!< The F key
    "G",            //!< The G key
    "H",            //!< The H key
    "I",            //!< The I key
    "J",            //!< The J key
    "K",            //!< The K key
    "L",            //!< The L key
    "M",            //!< The M key
    "N",            //!< The N key
    "O",            //!< The O key
    "P",            //!< The P key
    "Q",            //!< The Q key
    "R",            //!< The R key
    "S",            //!< The S key
    "T",            //!< The T key
    "U",            //!< The U key
    "V",            //!< The V key
    "W",            //!< The W key
    "X",            //!< The X key
    "Y",            //!< The Y key
    "Z",            //!< The Z key
    "0",         //!< The 0 key
    "1",         //!< The 1 key
    "2",         //!< The 2 key
    "3",         //!< The 3 key
    "4",         //!< The 4 key
    "5",         //!< The 5 key
    "6",         //!< The 6 key
    "7",         //!< The 7 key
    "8",         //!< The 8 key
    "9",         //!< The 9 key
    "Esc",       //!< The Escape key
    "LCtrl",     //!< The left Control key
    "LShift",       //!< The left Shift key
    "LAlt",         //!< The left Alt key
    "LWin",      //!< The left OS specific key: window (Windows and Linux), apple (macOS), ...
    "RCtrl",     //!< The right Control key
    "RShift",       //!< The right Shift key
    "RAlt",         //!< The right Alt key
    "RWin",      //!< The right OS specific key: window (Windows and Linux), apple (macOS), ...
    "Menu",         //!< The Menu key
    "[",     //!< The [ key
    "]",     //!< The ] key
    ";",    //!< The ; key
    ",",        //!< The , key
    ".",       //!< The . key
    "'",   //!< The ' key
    "/",        //!< The / key
    "\\",    //!< The \ key
    "`",        //!< The ` key
    "=",        //!< The = key
    "-",       //!< The - key (hyphen)
    "Space",        //!< The Space key
    "Enter",        //!< The Enter/Return keys
    "Bckspc",    //!< The Backspace key
    "Tab",          //!< The Tabulation key
    "PgUp",       //!< The Page up key
    "PgDn",     //!< The Page down key
    "End",          //!< The End key
    "Home",         //!< The Home key
    "Insert",       //!< The Insert key
    "Del",       //!< The Delete key
    "+",          //!< The + key
    "-",     //!< The - key (minus, usually from numpad)
    "*",     //!< The * key
    "/",       //!< The / key
    "Left",         //!< Left arrow
    "Right",        //!< Right arrow
    "Up",           //!< Up arrow
    "Down",         //!< Down arrow
    "Num0",      //!< The numpad 0 key
    "Num1",      //!< The numpad 1 key
    "Num2",      //!< The numpad 2 key
    "Num3",      //!< The numpad 3 key
    "Num4",      //!< The numpad 4 key
    "Num5",      //!< The numpad 5 key
    "Num6",      //!< The numpad 6 key
    "Num7",      //!< The numpad 7 key
    "Num8",      //!< The numpad 8 key
    "Num9",      //!< The numpad 9 key
    "F1",           //!< The F1 key
    "F2",           //!< The F2 key
    "F3",           //!< The F3 key
    "F4",           //!< The F4 key
    "F5",           //!< The F5 key
    "F6",           //!< The F6 key
    "F7",           //!< The F7 key
    "F8",           //!< The F8 key
    "F9",           //!< The F9 key
    "F10",          //!< The F10 key
    "F11",          //!< The F11 key
    "F12",          //!< The F12 key
    "F13",          //!< The F13 key
    "F14",          //!< The F14 key
    "F15",          //!< The F15 key
    "Pause",        //!< The Pause key
};

void initRumble(){}

void quit(){
    window.close();
    deinitialize();
    freeAudio();

    exit(0);
}

#endif
