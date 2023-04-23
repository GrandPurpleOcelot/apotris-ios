#pragma once

#include <list>
#include "platform.hpp"
#include <string>
#include "font1tiles_bin.h"

#define MAX_WORD_SPRITES 15

extern int previousSelection;

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
        virtual void control() = 0;
        virtual void init() = 0;
        virtual void deinit() = 0;

    Scene(){};

    virtual ~Scene(){};
};

extern Scene * scene;

class GameScene : public Scene{
    void draw();
    void update(){};
    void control(){};
    void init(){};
    void deinit(){};
};

class TitleScene : public Scene{
    void draw();
    void update(){};
    void control(){};
    void init(){};
    void deinit(){};
};

class GraphicsScene : public Scene{
    void draw();
    void update(){};
    void control(){};
    void init(){};
    void deinit(){};
};

class EditorScene : public Scene{
    void draw();
    void update(){};
    void control(){};
    void init(){};
    void deinit(){};
};

class newTitleScene : public Scene{
    WordSprite* wordSprites[MAX_WORD_SPRITES];

    WordSprite versionText = WordSprite(0,20,2);
    WordSprite nameText = WordSprite(0,23,2 + 12);

    OBJ_ATTR * cursorSprites[2];

    int flashTimer = 0;
    const int maxFlash = 48;

    void draw();
    void update();
    void control();
    void init();
    void deinit();

    ~newTitleScene(){
        deinit();
    };
};

class MainMenuScene : public Scene{
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

    OBJ_ATTR * cursorSprites[2];

    int cursorFloat = 0;

    void draw();
    void update();
    void control();
    void init();
    void deinit();

    ~MainMenuScene(){
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
    void control();
    void init();
    void deinit();

    ~SettingsScene(){
        deinit();
    };
};

extern void changeScene(Scene * newScene);
extern void changeScene(Scene * newScene,Transitions transition);

// class Option{
//     public:
//         std::string text;

//         Option(std::string _text){
//             text = _text;
//         }
// };

// class Element{
//     public:
//         int x = 0;
//         int y = 0;

//         std::string text = "Lorem Ipsum";

//         std::list<Option> options;

//         Element(int _x, int _y){
//             x = _x;
//             y = _y;

//             options.push_back(Option("ON"));
//             options.push_back(Option("OFF"));
//         }

//         void onPress(int dir){
//             log("Press! " + std::to_string(dir));
//         }

//         void draw(){
//             x/=2;
//             naprint(text, x, y);

//         }
// };

// class Menu{
//     public:
//         void init();
//         void deinit();
//         void draw();

//         void control(){
//             if(key_hit(KEY_DOWN)){
//                 sfx(SFX_MENUMOVE);

//                 if(++selector >= (int) elements.size()){
//                     selector = 0;
//                 }
//             }

//             if(key_hit(KEY_UP)){
//                 sfx(SFX_MENUMOVE);

//                 if(--selector < 0){
//                     selector = elements.size()-1;
//                 }
//             }

//             if(key_hit(KEY_A)){
//                 auto it = elements.begin();

//                 std::advance(it,selector);

//                 it->onPress(1);
//             }
//         };

//         void update(){
//             control();
//         };

//         std::list<Element> elements;

//         int selector = 0;

//         Menu(){
//             elements.push_back(Element(SCREEN_WIDTH/2, SCREEN_HEIGHT/2));

//             for(auto element : elements){
//                 element.draw();
//             }
//         }
// };
