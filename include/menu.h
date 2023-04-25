#pragma once

#include <list>
#include "platform.hpp"
#include <string>
#include "font1tiles_bin.h"

#define MAX_WORD_SPRITES 15

extern int previousSelection;

extern void sceneSwitcher(std::string str);
extern const std::list<std::string> menuOptions;
extern const std::list<std::string> gameOptions;

enum Transitions{
    INSTANT,
    FADE,
    SCANLINE
};

class WordSprite {
public:
    std::string text = "";
    int startIndex;
    int startTiles;
    int id;
    int priority = 0;
    bool blend = false;
    OBJ_ATTR* sprites[3];

    void show(int x, int y, int palette) {
        for (int i = 0; i < 3; i++) {
            sprite_unhide(sprites[i], 0);
            sprite_set_attr(sprites[i], ShapeWide, 1, startTiles+i*4, palette, priority);

            if(blend)
                sprite_enable_blend(sprites[i]);

            sprite_set_pos(sprites[i], x + i * 32, y);
        }
    }

    void show(int x, int y, int palette, FIXED scale) {
        for (int i = 0; i < 3; i++) {
            int affId = id*3+i;
            sprite_unhide(sprites[i], 0);
            sprite_set_attr(sprites[i], ShapeWide, 1, startTiles+i*4, palette, priority);

            if(blend)
                sprite_enable_blend(sprites[i]);

            sprite_set_pos(sprites[i], x + fx2int(fxdiv(int2fx( i * 32 ),scale)), y);
            sprite_set_size(sprites[i], scale, affId);
            sprite_enable_affine(sprites[i], affId, true);
        }
    }

    void setPriority(int newPriority){
        priority = newPriority;
    }

    void hide() {
        for (int i = 0; i < 3; i++)
            sprite_hide(sprites[i]);
    }

    void enableBlend(bool state){
        blend = state;
    }

    void setText(std::string _text) {
        if (_text == text)
            return;

        text = _text;


        if(text == ""){
            clearSpriteTiles(startTiles, 12, 0);
            // memset32(&tile_mem[4][startTiles], 0, 12 * 8);
            return;
        }

        // clearSpriteTiles(startTiles, 12, 1);

        // naprintSprite(text,startTiles);

        int n = min((int)text.size(),12);

        TILE* font = (TILE*)font1tiles_bin;
        int i;
        for (i = 0; i < n; i++) {
            int c = text[i] - 32 - 1;

            if(c >= 0)
                setSpriteTile(startTiles,i,0,0,&font[c]);
            // memcpy32(&tile_mem[4][startTiles + i], &font[c], 8);
            // dma3_cpy(&tile_mem[4][startTiles + i], &font[c], 32);
        }

        if(i < 12){

            //TODO:
            //"+ i" might be wrong
            clearSpriteTiles(startTiles + i, 12-i, 0);
            // memset32(&tile_mem[4][startTiles + i], 0, (12-i) * 8);
        }
    }

    WordSprite(int _id,int _index, int _tiles) {
        id = _id;
        startIndex = _index;
        startTiles = _tiles;

        for (int i = 0; i < 3; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }
};

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
        int startX = 0;
        int endX = 0;
        int y = 0;

        std::string label;
        std::list<std::string> options;
        int cursorType = 0;

        void showLabel();
        void showOption();

        void showCursor(bool state);
        virtual void control(int dir){};
        virtual void setLabel(){};
        virtual void setOptions(){};

        // Element(){
            // setLabel();
            // setOptions();
        // }

        // ~Element(){}
};

class TestElement : public Element{
    public:
        void control(int dir);
        // void setLabel(){label = "test";};
        // void setLabel();
        // void setOptions(){
        //     options.push_back("test1");
        //     options.push_back("test2");
        //     options.push_back("test3");
        // };

        TestElement(){
            // Element();
            label = "test";
            options.push_back("test1");
            options.push_back("test2");
            options.push_back("test3");
            // Element();
            // setLabel();
            // setOptions();
        }

        // ~TestElement(){}
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

    virtual std::list<std::string> getOptionList(){return menuOptions;};
    std::list<std::string> optionList;

    virtual std::string name(){return "Option";};

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

    ~OptionListScene(){
        deinit();
    };

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


extern void changeScene(Scene * newScene);
extern void changeScene(Scene * newScene,Transitions transition);

