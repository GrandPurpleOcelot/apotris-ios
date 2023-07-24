#pragma once

#include "def.h"
#include "tetrisEngine.h"
#include <list>
#include <string>
#include <tuple>
#include <map>
#include <algorithm>

extern int previousSelection;

extern void sceneSwitcher(std::string str);
extern const std::list<std::string> menuOptions;
extern const std::list<std::string> gameOptions;
extern const std::list<std::string> settingsOptions;
extern const std::list<std::string> controlOptions;
extern std::map<int,std::string> keyMap;

class Scene{
public:
        virtual void draw() = 0;
        virtual void update() = 0;
        virtual bool control() = 0;
        virtual void init() = 0;
        virtual void deinit() = 0;
        virtual Scene* previousScene() = 0;

    Scene(){};

    virtual ~Scene(){};
};

extern Scene * scene;

class GameScene : public Scene{
    void draw();
    void update();
    bool control();
    void init();
    void deinit();

    void countdown();

    bool counted = false;

    Scene* previousScene(){return NULL;};

    ~GameScene(){
        deinit();
    };
};

class TitleScene : public Scene{
    void draw();
    void update(){};
    bool control(){return false;};
    void init(){};
    void deinit(){};
    Scene* previousScene(){return NULL;};
};

class EditorScene : public Scene{
    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return NULL;};

    int currentColor = 2;
    int currentTool = 1;

    u8 das = 0;
    const int arr = 4;
    const int dasMax = 20;

    u8 timer = 0;
};

class newTitleScene : public Scene{
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite versionText = WordSprite(0,20,2);
    WordSprite nameText = WordSprite(0,23,2 + 12);

    OBJ_ATTR * cursorSprites[2];

    int flashTimer = 0;
    const int flashMax = 48;

    int demoTimer = 0;
    const int demoMax = 60 * 15;

    bool testDraw = false;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return NULL;};

    ~newTitleScene(){
        deinit();
    };
};

class SimpleListScene : public Scene{
    public:
    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool moving = false;
    bool movingHor = false;
    int movingTimer = 0;
    int movingDirection = 0;

    virtual std::list<std::string> getOptionList(){return menuOptions;};
    std::list<std::string> optionList;

    virtual std::string name(){return "";};

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    OBJ_ATTR * cursorSprites[2];

    OBJ_ATTR * arrowSprites[2];

    int cursorFloat = 0;

    int listStart = 0;
    const int elementMax = 4;

    int startY = 0;

    bool refreshText = true;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return NULL;};

    ~SimpleListScene(){
        deinit();
    };

};

class MainMenuScene: public SimpleListScene{
    public:
    void init();
    Scene* previousScene();

    std::list<std::string> getOptionList(){return menuOptions;};
    std::string name(){return "";};
};

class ModeListScene: public SimpleListScene{
    public:
    Scene* previousScene();

    std::list<std::string> getOptionList(){return gameOptions;};
    std::string name(){return "Play";};
};

class Element{
    public:

        virtual std::string getLabel(){return "";};
        virtual std::string getCursor(std::string text){return "";};
        virtual void action(int dir){};
        virtual void action(){};
        virtual int getValue(){return 0;};
        virtual std::string getCurrentOption(){return "";};

        Element(){}

        virtual ~Element(){}
};

class MusicVolumeElement : public Element{
    public:
        std::string getLabel() override {
            return "Music";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.volume > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.volume < 10)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if(savefile->settings.volume < 10)
                    savefile->settings.volume++;
            }else{
                if(savefile->settings.volume > 0)
                    savefile->settings.volume--;
            }
            setMusicVolume(512 * ((float)savefile->settings.volume / 10));
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.volume);
        }
};

class SFXVolumeElement : public Element{
    public:
        std::string getLabel() override {
            return "SFX";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.sfxVolume > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.sfxVolume < 10)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if(savefile->settings.sfxVolume < 10)
                    savefile->settings.sfxVolume++;
            }else{
                if(savefile->settings.sfxVolume > 0)
                    savefile->settings.sfxVolume--;
            }
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.sfxVolume);
        }
};

class AnnouncerElement : public Element{
    public:
        std::string getLabel() override {
            return "Announcer";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.announcer = !savefile->settings.announcer;
            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            return (savefile->settings.announcer) ? "ON" : "OFF";
        }
};

class PlaybackElement : public Element{
    public:
        std::string getLabel() override {
            return "Playback";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.cycleSongs = !savefile->settings.cycleSongs;
            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            return (savefile->settings.cycleSongs) ? "CYCLE" : "LOOP";
        }
};

class SongListElement : public Element{
    public:
        std::string getLabel() override {
            return "Song List";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override{
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            return "...";
        }
};

class DASElement : public Element{
    public:

        std::string getLabel() override {
            return "DAS";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(!(savefile->settings.das == 1 && savefile->settings.customDas))
                result += "<";
            else
                result += " ";

            result += text;

            if(!(savefile->settings.das == 8 && !savefile->settings.customDas))
                result += ">";

            return result;
        }

        void action(int dir) override{
            int n = 0;

            if(savefile->settings.customDas){
                n = savefile->settings.das-30;
            }else{
                switch(savefile->settings.das){
                    case 16: n = 1; break;
                    case 11: n = 2; break;
                    case  9: n = 3; break;
                    case  8: n = 4; break;
                }
            }

            if(n + dir <= 4 && n + dir >= -29)
                n += dir;

            if(n > 0){
                switch(n){
                    case 1: n = 16; break;
                    case 2: n = 11; break;
                    case 3: n = 9; break;
                    case 4: n = 8; break;
                }
                savefile->settings.customDas = false;
                savefile->settings.das = n;
            }else{
                n = 30 + n;
                savefile->settings.customDas = true;
                savefile->settings.das = n;
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            int n = savefile->settings.das;

            if(savefile->settings.customDas)
                return std::to_string(n) + "f";

            switch(savefile->settings.das){
                case 16: return "SLOW"; break;
                case 11: return "MID"; break;
                case 9: return "FAST"; break;
                case 8: return "V.FAST"; break;
                default: return "ERROR"; break;
            }
        }
};

class ARRElement : public Element{
    public:
        std::string getLabel() override {
            return "ARR";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.arr < 3)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.arr > -1)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if(savefile->settings.arr > -1)
                    savefile->settings.arr--;
            }else{
                if(savefile->settings.arr < 3)
                    savefile->settings.arr++;
            }
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.arr){
                case -1: return "INSTANT"; break;
                case 0: return "V.FAST"; break;
                case 1: return "FAST"; break;
                case 2: return "MID"; break;
                case 3: return "SLOW"; break;
                default: return "ERROR"; break;
            }
        }
};

class SDFElement : public Element{
    public:
        std::string getLabel() override {
            return "SDF";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.sfr < 3)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.sfr > -1)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if(savefile->settings.sfr > -1)
                    savefile->settings.sfr--;
            }else{
                if(savefile->settings.sfr < 3)
                    savefile->settings.sfr++;
            }
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.sfr){
                case -1: return "INSTANT"; break;
                case 0: return "V.FAST"; break;
                case 1: return "FAST"; break;
                case 2: return "MID"; break;
                case 3: return "SLOW"; break;
                default: return "ERROR"; break;
            }
        }
};

class DelaySoftDropElement : public Element{
    public:
        std::string getLabel() override {
            return "Delay Soft Drop";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.delaySoftDrop = !savefile->settings.delaySoftDrop;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.delaySoftDrop)
                return "ON";
            else
                return "OFF";
        }
};

class DropProtectionElement : public Element{
    public:
        std::string getLabel() override {
            return "Drop Protection";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.dropProtectionFrames > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.dropProtectionFrames < 20)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if(savefile->settings.dropProtectionFrames < 20)
                    savefile->settings.dropProtectionFrames++;
            }else{
                if(savefile->settings.dropProtectionFrames > 0)
                    savefile->settings.dropProtectionFrames--;
            }
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.dropProtectionFrames);
        }
};

class DirectionalDelayElement : public Element{
    public:
        std::string getLabel() override {
            return "Directional Delay";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.directionalDas = !savefile->settings.directionalDas;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.directionalDas)
                return "ON";
            else
                return "OFF";
        }
};

class DisableDiagonalsElement : public Element{
    public:
        std::string getLabel() override {
            return "Disable Diagonals";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.diagonalType < 2)
                    savefile->settings.diagonalType++;
            }else{
                if(savefile->settings.diagonalType > 0)
                    savefile->settings.diagonalType--;
            }
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.diagonalType){
            case 0: return "OFF"; break;
            case 1: return "SOFT"; break;
            case 2: return "STRICT"; break;
            default: return "ERROR"; break;
            }
        }
};

class IHSElement : public Element{
    public:
        std::string getLabel() override {
            return "Initial Hold";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.ihs = !savefile->settings.ihs;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.ihs)
                return "ON";
            else
                return "OFF";
        }
};

class IRSElement : public Element{
    public:
        std::string getLabel() override {
            return "Initial Rotation";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.irs = !savefile->settings.irs;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.irs)
                return "ON";
            else
                return "OFF";
        }
};

class InitialSystemElement : public Element{
    public:
        std::string getLabel() override {
            return "Initial System";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.initialType = !savefile->settings.initialType;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.initialType)
                return "TYPE B";
            else
                return "TYPE A";
        }
};

class ABHoldElement : public Element{
    public:
        std::string getLabel() override {
            return "A+B to Hold";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.abHold = !savefile->settings.abHold;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.abHold)
                return "ON";
            else
                return "OFF";
        }
};

class QuickResetElement : public Element{
    public:
        std::string getLabel() override {
            return "Quick Reset";
        }

        std::string getCursor(std::string text) override {
            std::string result;
            if(!savefile->settings.resetHoldToggle)
                result += "<";
            else
                result += " ";

            result += text;

            if(!savefile->settings.resetHoldType)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.resetHoldToggle){
                    savefile->settings.resetHoldToggle = 0;
                }else if(!savefile->settings.resetHoldType){
                    savefile->settings.resetHoldType = 1;
                }
            }else{
                if(!savefile->settings.resetHoldType){
                    savefile->settings.resetHoldToggle = 1;
                }else if(savefile->settings.resetHoldType){
                    savefile->settings.resetHoldType = 0;
                }
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.resetHoldToggle)
                return "OFF";

            if(savefile->settings.resetHoldType)
                return "HOLD";

            return "PRESS";
        }
};

class RumbleElement : public Element{
    public:
        std::string getLabel() override {
            return "Rumble";
        }

        std::string getCursor(std::string text) override {
            std::string result;
            if(savefile->settings.rumble > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.rumble < 4)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.rumble < 4)
                    savefile->settings.rumble++;
            }else{
                if(savefile->settings.rumble > 0)
                    savefile->settings.rumble--;
            }
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.rumble * 25) + "%";
        }
};

class ResetControlsElement : public Element{
    public:
        std::string getLabel() override {
            return "Reset Controls";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override{
            Keys k;
            k.moveLeft = KEY_LEFT;
            k.moveRight = KEY_RIGHT;
            k.rotateCW = KEY_A;
            k.rotateCCW = KEY_B;
            k.rotate180 = 0;
            k.softDrop = KEY_DOWN;
            k.hardDrop = KEY_UP;
            k.hold = KEY_R | KEY_L;
            k.zone = KEY_SELECT;

            savefile->settings.keys = k;
        }

        std::string getCurrentOption() override {
            return "";
        }
};

class RebindElement : public Element{
    public:
        std::string label;
        int index = 0;

        bool reading = false;

        std::string getLabel() override {
            return label;
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override{
            int* keys = (int*) &savefile->settings.keys;
            std::list<int> foundKeys;

            reading = true;

            while(closed()){
                canDraw = 1;
                vsync();
                key_poll();

                if(key_hit(KEY_START)){
                    sfx(SFX_MENUCANCEL);
                    break;
                }

                u16 key = key_hit(KEY_FULL);

                if(key != 0){
                    std::list<int> currentKeys;

                    int temp = key;

                    int counter = 0;
                    do{
                        if(temp & (1 << counter)){
                            currentKeys.push_back(1<<counter);
                            temp-= 1 << counter;
                        }

                        counter++;
                    }while(temp != 0);

                    for(int i = 0; i < 9; i++){
                        foundKeys.clear();

                        int k = keys[i];
                        counter = 0;
                        do{
                            if(k & (1 << counter)){
                                foundKeys.push_back(1<<counter);
                                k-= 1 << counter;
                            }

                            counter++;
                        }while(k != 0);

                        for(auto const & cur : currentKeys){
                            if(std::find(foundKeys.begin(), foundKeys.end(),(int) cur) != foundKeys.end())
                                keys[i]-=cur;
                        }
                    }

                    keys[index] = key;
                    break;
                }
            }

            reading = false;
        }

        std::string getCurrentOption() override {
            if(reading){
                return "...";
            }

            int *keys = (int *) &savefile->settings.keys;
            return keyMap[keys[index]];
        }

        RebindElement(int _index, std::string _label){
            index = _index;
            label = _label;
        }
};

class GradientElement : public Element{
    public:
        std::string getLabel() override {
            return "Background";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override;

        std::string getCurrentOption() override {
            return "...";
        }
};

class GridElement : public Element{
    public:
        std::string getLabel() override {
            return "Grid";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.backgroundGrid > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if(savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1)
                    savefile->settings.backgroundGrid++;
            }else{
                if(savefile->settings.backgroundGrid > 0)
                    savefile->settings.backgroundGrid--;
            }
            drawGrid();
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.backgroundGrid);
        }
};

class SkinElement : public Element{
    public:
        std::string getLabel() override {
            return "Skin";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.skin > -(MAX_CUSTOM_SKINS))
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.skin < MAX_SKINS - 1)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if (savefile->settings.skin < MAX_SKINS - 1)
                    savefile->settings.skin++;
            }else{
                if (savefile->settings.skin > -(MAX_CUSTOM_SKINS))
                    savefile->settings.skin--;
            }
            setSkin();
        }

        std::string getCurrentOption() override {
            if(savefile->settings.skin >= 0){
                return std::to_string(savefile->settings.skin);
            }else{
                return "C" + std::to_string(- (savefile->settings.skin));
            }
        }
};

class GhostElement : public Element{
    public:
        std::string getLabel() override {
            return "Ghost Piece";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.shadow > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.shadow < MAX_SHADOWS - 1)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if (savefile->settings.shadow < MAX_SHADOWS - 1)
                    savefile->settings.shadow++;
            }else{
                if (savefile->settings.shadow > 0)
                    savefile->settings.shadow--;
            }
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.shadow);
        }
};

class FrameColorElement : public Element{
    public:
        std::string getLabel() override {
            return "Frame Color";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.palette > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.palette < 7)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if (savefile->settings.palette < 7)
                    savefile->settings.palette++;
            }else{
                if (savefile->settings.palette > 0)
                    savefile->settings.palette--;
            }
            setPalette();
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.palette);
        }
};

class PaletteElement : public Element{
    public:
        std::string getLabel() override {
            return "Palette";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.colors > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.colors < MAX_COLORS - 1)
                result += ">";

            return result;
        }

        void action(int dir) override{
            sfx(SFX_MENUMOVE);
            if(dir > 0){
                if (savefile->settings.colors < MAX_COLORS - 1)
                    savefile->settings.colors++;
            }else{
                if (savefile->settings.colors > 0)
                    savefile->settings.colors--;
            }

            setPalette();
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.colors);
        }
};

class OptionListScene : public Scene{
    public:
    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool moving = false;
    bool movingHor = false;
    int movingTimer = 0;
    int movingDirection = 0;

    virtual std::list<Element*> getElementList() = 0;
    std::list<Element*> elementList;

    virtual std::string name() = 0;

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite* labels[7];

    WordSprite cursorText = WordSprite(0,20,2);

    OBJ_ATTR * cursorSprites[2];

    OBJ_ATTR * arrowSprites[2];

    int cursorFloat = 0;

    bool refreshText = true;

    int listStart = 0;
    const int elementMax = 5;

    int startY = 0;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    virtual Scene* previousScene(){return NULL;};

    ~OptionListScene(){
        deinit();
    };
};

class StatListScene : public Scene{
    public:
    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool moving = false;
    bool movingHor = false;
    int movingTimer = 0;
    int movingDirection = 0;

    virtual std::list<Element*> getElementList() = 0;
    std::list<Element*> elementList;

    virtual std::string name() = 0;

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite* labels[7];

    WordSprite cursorText = WordSprite(0,20,2);

    OBJ_ATTR * cursorSprites[2];

    OBJ_ATTR * arrowSprites[2];

    int cursorFloat = 0;

    bool refreshText = true;

    int listStart = 0;
    const int elementMax = 5;

    int startY = 0;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    virtual Scene* previousScene(){return NULL;};

    ~StatListScene(){
        deinit();
    };
};

class ConfirmSaveScene : public Scene{
    public:

    int timer = 0;

    bool cancel = true;

    int cursorFloat = 0;

    OBJ_ATTR * cursorSprites[2];

    int lengths[2];

    int pos[2];

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return new MainMenuScene();};
};

class SettingsScene : public SimpleListScene{
    public:
        Scene* previousScene(){
            if(settingsChanged())
                return new ConfirmSaveScene();
            else
                return new MainMenuScene();
        };

        std::list<std::string> getOptionList(){return settingsOptions;};
        std::string name(){return "Settings";};
};

class GraphicsScene : public Scene{
    void draw(){};
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return new SettingsScene();};

    bool showOptions = true;
    bool showGame = true;

    int selection = 0;

    const int maxDas = 16;
    int dasVer = 0;
    int dasHor = 0;
    int dasOffset = 0;

    const int maxArr = 3;
    int arr = 0;

    int prevBld = blendInfo;
};

class newGraphicsScene : public Scene{
    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool moving = false;
    bool movingHor = false;
    int movingTimer = 0;
    int movingDirection = 0;

        std::list<Element*> getElementList(){
            return {
                new GradientElement(),
                new GridElement(),
                new SkinElement(),
                new GhostElement(),
                new FrameColorElement(),
                new PaletteElement(),
            };
        };
    std::list<Element*> elementList;

    std::string name() {return "Graphics";};

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite* labels[7];

    WordSprite cursorText = WordSprite(0,20,2);

    OBJ_ATTR * cursorSprites[2];

    int cursorFloat = 0;

    bool refreshText = true;
    bool refreshGame = false;

    bool showText = true;

    int listStart = 0;
    const int elementMax = 5;

    int startY = 0;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    virtual Scene* previousScene(){return new SettingsScene();};

    ~newGraphicsScene(){
        deinit();
    };
};


class AudioOptionScene : public OptionListScene{
    public:
    std::string name(){return "Audio";};
        std::list<Element*> getElementList(){
            return {
                new MusicVolumeElement(),
                new SFXVolumeElement(),
                new AnnouncerElement(),
                new PlaybackElement(),
                new SongListElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};

class ControlOptionScene : public OptionListScene{
    public:
    std::string name(){return "Controls";};

    std::list<Element*> getElementList(){
        std::list<Element*> list;

        auto it = controlOptions.begin();

        int counter = 0;
        while(it != controlOptions.end()){
            list.push_back(new RebindElement(counter++,*it));

            it++;
        }

        list.push_back(new ABHoldElement());
        list.push_back(new QuickResetElement());
        list.push_back(new RumbleElement());
        list.push_back(new ResetControlsElement());

        return list;
    };

    Scene* previousScene(){return new SettingsScene();};
};

class HandlingOptionScene : public OptionListScene{
    public:
    std::string name(){return "Handling";};
        std::list<Element*> getElementList(){
            return {
                new DASElement(),
                new ARRElement(),
                new SDFElement(),
                new DelaySoftDropElement(),
                new DropProtectionElement(),
                new DirectionalDelayElement(),
                new DisableDiagonalsElement(),
                new IHSElement(),
                new IRSElement(),
                new InitialSystemElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};

class PlayOptionScene : public OptionListScene{
    public:

    int subMode = 0;
    int goal = 0;
    int level = 0;

    virtual Tetris::Modes getMode() = 0;

    bool control();

    virtual void start();

    Scene* previousScene(){return new ModeListScene();};
};

class PlayTimeElement : public Element{
    public:
        std::string getLabel() override {
            return "Play Time";
        }

        std::string getCursor(std::string text) override {
            return " " + text + " ";
        }

        std::string getCurrentOption() override {
            return timeToStringHours(savefile->stats.timePlayed);
        }
};

class GamesStartedElement : public Element{
    public:
        std::string getLabel() override {
            return "Games Started";
        }

        std::string getCursor(std::string text) override {
            return " " + text + " ";
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->stats.gamesStarted);
        }
};

class GamesCompletedElement : public Element{
    public:
        std::string getLabel() override {
            return "Games Completed";
        }

        std::string getCursor(std::string text) override {
            return " " + text + " ";
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->stats.gamesCompleted);
        }
};

class StatScene : public OptionListScene{
    public:
        std::string name(){return "Stats";};

        std::list<Element*> getElementList(){
            return {
                new PlayTimeElement(),
                new GamesStartedElement(),
                new GamesCompletedElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};

class LevelSelectorElement : public Element{
    public:
        int value = 1;

        int min = 0;
        int max = 1;

        int getValue() override{
            return value;
        }

        std::string getLabel() override {
            return "Level";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 1)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < 20)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(value < 20)
                    value++;
            }else{
                if(value > 1)
                    value--;
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            return std::to_string(value);
        }

        LevelSelectorElement(int _min, int _max){
            min = _min;
            max = _max;
        }
};

class StringSelectorElement : public Element{
    public:
        int value = 0;
        int options = 1;

        std::list<std::string> optionList;
        std::string label;

        virtual int getValue() override {return value;};

        std::string getLabel() override {return label;};

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < options - 1)
                result += ">";

            return result;
        }

        void action(int dir) override {
            if(dir > 0){
                if(value < options - 1)
                    value++;
            }else{
                if(value > 0)
                    value--;
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            if(value > (int) optionList.size())
                return "";

            auto it = optionList.begin();
            std::advance(it,value);

            return *it;
        }

        StringSelectorElement(std::list<std::string> list){
            optionList = list;
            options = optionList.size();
        }

        StringSelectorElement(std::string newLabel, std::list<std::string> list){
            optionList = list;
            options = optionList.size();
            label = newLabel;
        }

};

class IntSelectorElement : public Element{
    public:
        int value = 0;
        int options = 1;

        std::list<int> optionList;
        std::string label;

        virtual int getValue() override {
            if(value > (int) optionList.size())
                return 0;

            auto it = optionList.begin();
            std::advance(it,value);

            return *it;
        };

        std::string getLabel() override {return label;};

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < options - 1)
                result += ">";

            return result;
        }

        void action(int dir) override {
            if(dir > 0){
                if(value < options - 1)
                    value++;
            }else{
                if(value > 0)
                    value--;
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            if(value > (int) optionList.size())
                return "";

            auto it = optionList.begin();
            std::advance(it,value);

            return std::to_string(*it);
        }

        IntSelectorElement(std::list<int> list){
            optionList = list;
            options = optionList.size();
        }

        IntSelectorElement(std::string newLabel, std::list<int> list){
            optionList = list;
            options = optionList.size();
            label = newLabel;
        }

        IntSelectorElement(std::string newLabel, std::list<int> list, int _value){
            optionList = list;
            options = optionList.size();
            label = newLabel;
            value = _value;
        }
};

class TupleSelectorElement : public Element{
    public:
        int value = 0;
        int options = 1;

        std::list<std::tuple<int,std::string>> optionList;
        std::string label;

        std::string getLabel() override {return label;};

        int getValue() override {
            if(value > (int) optionList.size())
                return value;

            auto it = optionList.begin();
            std::advance(it,value);

            return std::get<0>(*it);
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < options - 1)
                result += ">";

            return result;
        }

        void action(int dir) override {
            if(dir > 0){
                if(value < options - 1)
                    value++;
            }else{
                if(value > 0)
                    value--;
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            if(value > (int) optionList.size())
                return "";

            auto it = optionList.begin();
            std::advance(it,value);

            return std::get<1>(*it);
        }

        TupleSelectorElement(std::list<std::tuple<int,std::string>> list){
            optionList = list;
            options = optionList.size();
        }

        TupleSelectorElement(std::string newLabel, std::list<std::tuple<int,std::string>> list){
            optionList = list;
            options = optionList.size();
            label = newLabel;
        }

};

class MarathonScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::MARATHON;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Zone"}),
            new LevelSelectorElement(1,20),
            new TupleSelectorElement("Lines",{{150,"150"},{200,"200"},{300,"300"},{0,"Endless"},}),
        };
    };

    std::string name(){return "Marathon";};
};

class SprintScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::SPRINT;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Attack"}),
            new IntSelectorElement("Lines",{20,40,100},1),
        };
    };

    std::string name(){return "Sprint";};
};

class DigScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::DIG;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Efficiency"}),
            new IntSelectorElement("Lines",{10,20,100}),
        };
    };

    std::string name(){return "Dig";};
};

class UltraScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::ULTRA;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Minutes",{{3 * 3600,"3"},{5 * 3600,"5"},{10* 3600, "10"}}),
        };
    };

    std::string name(){return "Ultra";};
};

class BlitzScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::BLITZ;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    std::string name(){return "Blitz";};

    BlitzScene(){
        goal = 2 * 3600;
    }
};

class ComboScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::COMBO;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    std::string name(){return "Combo";};
};

class SurvivalScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::SURVIVAL;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Difficulty",{{1,"EASY"},{2,"MEDIUM"},{3,"HARD"}}),
        };
    };

    std::string name(){return "Survival";};
};

class ClassicScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::CLASSIC;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"A-TYPE","B-TYPE"}),
            new LevelSelectorElement(0,19),
        };
    };

    std::string name(){return "Classic";};
};

class GradientEditorScene : public Scene{
    int color[3];

    int selection = 0;

    int das = 0;

    const int dasMax = 10;
    const int arrMax = 4;

    int c = 0;

    bool refreshText = false;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return new newGraphicsScene();};

    ~GradientEditorScene(){
        deinit();
    }
};

enum class Transitions{
    INSTANT,
    FADE,
    SCANLINE
};

extern void changeScene(Scene * newScene);
extern void changeScene(Scene * newScene,Transitions transition);

