#pragma once

#include "scene.hpp"

#ifdef PC
#include "liba_pc.h"
#endif

class IntegerScaleElement : public Element{
    public:
        std::string getLabel() override {
            return "Integer Scale";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.integerScale = !savefile->settings.integerScale;
            sfx(SFX_MENUCONFIRM);

            #ifdef PC
            refreshWindowSize();
            #endif
        }

        std::string getCurrentOption() override {
            if(savefile->settings.integerScale)
                return "ON";
            else
                return "OFF";
        }
};

class ZoomElement : public Element{
    public:
        std::string getLabel() override {
            return "Zoom";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.zoom > 12)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.zoom < 20)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.zoom < 20){
                    savefile->settings.zoom++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.zoom > 12){
                    savefile->settings.zoom--;
                    sfx(SFX_MENUMOVE);
                }
            }
            #ifdef PC
            refreshWindowSize();
            #endif
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.zoom * 5) + "%";
        }
};

class WindowOptionScene : public OptionListScene{
    public:
    std::string name(){return "Video";};
        std::list<Element*> getElementList(){
            return {
                new IntegerScaleElement(),
                new ZoomElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};
