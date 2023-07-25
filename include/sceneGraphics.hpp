#pragma once

#include "scene.hpp"

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
            if(dir > 0){
                if(savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1){
                    savefile->settings.backgroundGrid++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.backgroundGrid > 0){
                    savefile->settings.backgroundGrid--;
                    sfx(SFX_MENUMOVE);
                }
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
            if(dir > 0){
                if (savefile->settings.skin < MAX_SKINS - 1){
                    savefile->settings.skin++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if (savefile->settings.skin > -(MAX_CUSTOM_SKINS)){
                    savefile->settings.skin--;
                    sfx(SFX_MENUMOVE);
                }
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
            if(dir > 0){
                if (savefile->settings.shadow < MAX_SHADOWS - 1){
                    savefile->settings.shadow++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if (savefile->settings.shadow > 0){
                    savefile->settings.shadow--;
                    sfx(SFX_MENUMOVE);
                }
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
            if(dir > 0){
                if (savefile->settings.palette < 7){
                    savefile->settings.palette++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if (savefile->settings.palette > 0){
                    savefile->settings.palette--;
                    sfx(SFX_MENUMOVE);
                }
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
            if(dir > 0){
                if (savefile->settings.colors < MAX_COLORS - 1){
                    savefile->settings.colors++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if (savefile->settings.colors > 0){
                    savefile->settings.colors--;
                    sfx(SFX_MENUMOVE);
                }
            }

            setPalette();
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.colors);
        }
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

    int pathCount = 0;

    std::string p = "";

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
