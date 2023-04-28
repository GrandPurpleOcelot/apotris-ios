#pragma once

#include "audio_files.h"
#include "def.h"
#include <list>
#include <string>

extern int previousSelection;

extern void sceneSwitcher(std::string str);
extern const std::list<std::string> menuOptions;
extern const std::list<std::string> gameOptions;

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
    void update(){};
    bool control();
    void init(){};
    void deinit(){};
    Scene* previousScene(){return NULL;};
};

class TitleScene : public Scene{
    void draw();
    void update(){};
    bool control(){return false;};
    void init(){};
    void deinit(){};
    Scene* previousScene(){return NULL;};
};

class GraphicsScene : public Scene{
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

    virtual std::string name(){return "Menu";};

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    OBJ_ATTR * cursorSprites[2];

    int cursorFloat = 0;

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
    Scene* previousScene();

    std::list<std::string> getOptionList(){return menuOptions;};
    std::string name(){return "Menu";};
};

class ModeListScene: public SimpleListScene{
    public:
    Scene* previousScene();

    std::list<std::string> getOptionList(){return gameOptions;};
    std::string name(){return "Play";};
};

class Element{
    public:

        // std::string label;
        // std::list<std::string> options;
        // int cursorType = 0;

        // virtual void control(int dir){};
        // virtual void setLabel(){};
        // virtual void setOptions(){};

        virtual std::string getLabel(){return "";};
        // virtual std::list<std::string> getOptions(){return {};};
        virtual int getCursorType(){return 0;};
        virtual void action(int dir){};
        virtual std::string getCurrentOption(){return "";};

        Element(){
            // setLabel();
            // setOptions();
        }

        virtual ~Element(){}
};

// class TestElement : public Element{
//     public:
//         std::string getLabel() override {
//             return "test1";
//         }

//         // std::list<std::string> getOptions() override {
//         //     return {"option1","option2","option3"};
//         // }

//         int getCursorType() override {
//             return 0;
//         }

//         void action(int dir) override;

//         std::string getCurrentOption() override {
//             if(savefile->settings.effects)
//                 return "test3";
//             else
//                 return "test4";
//         }
// };

class MusicVolumeElement : public Element{
    public:
        std::string getLabel() override {
            return "Music";
        }

        int getCursorType() override {
            return 0;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.volume < 10)
                    savefile->settings.volume++;
            }else{
                if(savefile->settings.volume > 0)
                    savefile->settings.volume--;
            }

            sfx(SFX_MENUMOVE);
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

        int getCursorType() override {
            return 0;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.sfxVolume < 10)
                    savefile->settings.sfxVolume++;
            }else{
                if(savefile->settings.sfxVolume > 0)
                    savefile->settings.sfxVolume--;
            }

            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.sfxVolume);
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

    OBJ_ATTR * cursorSprites[2];

    int cursorFloat = 0;

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

class TestOption : public OptionListScene{
    public:
    std::string name(){return "Achievements";};
        std::list<Element*> getElementList(){
            return {
                new MusicVolumeElement(),
                new SFXVolumeElement(),
            };
        };
    Scene* previousScene(){return new MainMenuScene();};
};

class SettingsScene : public Scene{
    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool moving = false;
    bool movingHor = false;
    int movingTimer = 0;
    int movingDirection = 0;

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite saveText = WordSprite(0,20,2);

    OBJ_ATTR * cursorSprites[2];

    int cursorFloat = 0;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return NULL;};

    ~SettingsScene(){
        deinit();
    };
};

enum Transitions{
    INSTANT,
    FADE,
    SCANLINE
};

extern void changeScene(Scene * newScene);
extern void changeScene(Scene * newScene,Transitions transition);

