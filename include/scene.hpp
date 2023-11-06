#pragma once

#include "def.h"
#include "blockEngine.h"
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
extern const std::list<std::string> menuControlOptions;

extern std::string getDescription(std::string element);
extern std::string getDescription(std::string mode, std::string element, std::string option);

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

    WordSprite wordSprites[MAX_WORD_SPRITES];

    void countdown();
    void showText();
    void updateText();
    void showTimer();
    void showComboStreak();
    int pauseMenu();
    int endScreen();
    void showModeText();
    void showStats(bool, std::string, std::string, bool);

    bool counted = false;

    int timesPaused = 0;

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

class newTitleScene : public Scene{
    WordSprite wordSprites[MAX_WORD_SPRITES];

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

    std::string currentOption = "";
    std::string scrollText = "";

    int scrollTextLength = 0;

    int scrollTimer = 0;
    const int scrollTimerMax = 1;
    int scrollOffset = 0;
    int scrollDelay = 0;
    int scrollDelayMax = 60;

    int endDelay = 0;
    int endDelayMax = 80;
    WordSprite* scrollingText[3];

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

    std::list<std::string> getOptionList(){
        std::list<std::string> result = menuOptions;

        #if !(defined(GBA) || defined(WEB))

        result.push_back("Quit");

        #endif

        return result;
    };
    std::string name(){return "";};
};

class ModeListScene: public SimpleListScene{
    public:
    Scene* previousScene();

    std::list<std::string> getOptionList(){
        std::list<std::string> l = gameOptions;

        #ifdef PC
        l.remove("2P Battle");
        #endif

        return l;
    };
    std::string name(){return "Play";};
};

class Element{
    public:

        int value = 0;

        virtual std::string getLabel(){return "";};
        virtual std::string getCursor(std::string text){return "";};
        virtual void action(int dir){};
        virtual void action(){};
        virtual int getValue(){return 0;};
        virtual std::string getCurrentOption(){return "";};

        Element(){}

        virtual ~Element(){}
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

    std::string currentOption = "";
    std::string scrollText = "";

    int scrollTextLength = 0;

    int scrollTimer = 0;
    const int scrollTimerMax = 1;
    int scrollOffset = 0;
    int scrollDelay = 0;
    int scrollDelayMax = 60;

    int endDelay = 0;
    int endDelayMax = 80;
    WordSprite* scrollingText[3];

    int options = 0;
    int selection = 0;
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite* labels[7];

    WordSprite cursorText = WordSprite(0,20,2);

    OBJ_ATTR * cursorSprites[2];

    OBJ_ATTR * arrowSprites[2];

    int cursorFloat = 0;

    bool refreshText = true;
    bool refreshOption = true;

    int listStart = 0;
    const int elementMax = 5;

    int startY = 0;

    void showPath();

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
    void deinit(){};
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

        std::list<std::string> getOptionList(){
            std::list<std::string> result = settingsOptions;

            #ifndef GBA
            result.push_front("Video");
            #endif

            return result;
        };

        std::string name(){return "Settings";};
};

class CreditsScene : public Scene{
    public:

    virtual std::list<std::string> getOptionList(){return menuOptions;};
    std::list<std::string> optionList;

    std::string name(){return "Credits";};

    WordSprite* wordSprites[MAX_WORD_SPRITES];

    int listStart = 0;
    int maxShow = 12;

    int delay = 0;
    int delayMax = 60;

    int scrollTimer = 0;
    int scrollTimerMax = 4;
    int scrollOffset = 0;
    int space = 12;

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return new MainMenuScene();};

    ~CreditsScene(){
        deinit();
    };

};

class LinksScene : public SimpleListScene{

    public:
    // void init();
    Scene* previousScene(){return new MainMenuScene();};

    std::list<std::string> getOptionList(){
        return {
            "Website",
            "Wiki",
            "Donate",
            "Discord",
        };
    };

    std::string name(){return "Links";};
};

class QRScene : public Scene{
    public:

    virtual u8* getData() = 0;

    virtual std::string name() = 0;

    virtual std::string getLink() = 0;

    OBJ_ATTR * qr;

    int qrX = 0;
    int qrY = 0;

    WordSprite* wordSprites[MAX_WORD_SPRITES];

    void draw();
    void update();
    bool control();
    void init();
    void deinit();
    Scene* previousScene(){return new LinksScene();};

    ~QRScene(){
        deinit();
    };
};

#include "site_qr_bin.h"

class WebsiteLinkScene : public QRScene{
public:
    std::string name() override {
        return "Website";
    }

    u8* getData() override {
        return (u8*) site_qr_bin;
    }

    std::string getLink() override {
        return "https://apotris.com";
    }

    WebsiteLinkScene(){
        qrX = 29;
        qrY = 29;
    }
};

#include "wiki_qr_bin.h"

class WikiLinkScene : public QRScene{
public:
    std::string name() override {
        return "Wiki";
    }

    u8* getData() override {
        return (u8*) wiki_qr_bin;
    }

    std::string getLink() override {
        return "https://apotris.com/wiki";
    }

    WikiLinkScene(){
        qrX = 29;
        qrY = 29;
    }
};

#include "paypal_qr_bin.h"

class DonateLinkScene : public QRScene{
public:
    std::string name() override {
        return "Donate";
    }

    u8* getData() override {
        return (u8*) paypal_qr_bin;
    }

    std::string getLink() override {
        return "https://apotris.com/donate";
    }

    DonateLinkScene(){
        qrX = 29;
        qrY = 29;
    }
};

#include "discord_qr_bin.h"

class DiscordLinkScene : public QRScene{
public:
    std::string name() override {
        return "Discord";
    }

    u8* getData() override {
        return (u8*) discord_qr_bin;
    }

    std::string getLink() override {
        return "https://apotris.com/discord";
    }

    DiscordLinkScene(){
        qrX = 33;
        qrY = 33;
    }
};

enum class Transitions{
    INSTANT,
    FADE,
    SCANLINE
};

extern void changeScene(Scene * newScene);
extern void changeScene(Scene * newScene,Transitions transition);

