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

            if(savefile->settings.integerScale && savefile->settings.zoom > -1){
                savefile->settings.zoom = (savefile->settings.zoom / 10) * 10; //round to 10s
            }

            #if defined(PC) || defined(WEB) || defined(PORTMASTER) || defined(SWITCH)
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

class ScaleElement : public Element{
    public:

        const int min = -1;
        const int max = 90;
        std::string getLabel() override {
            return "Scale";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.zoom > min)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.zoom < max)
                result += ">";

            return result;
        }

        void action(int dir) override{
            int add = 1 + 9 * (savefile->settings.integerScale && savefile->settings.zoom > -1);
            if(dir > 0){
                if(savefile->settings.zoom < max){
                    savefile->settings.zoom+=add;

                    if(savefile->settings.zoom > max)
                        savefile->settings.zoom = max;

                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.zoom > min){
                    savefile->settings.zoom-=add;

                    if(savefile->settings.zoom < min)
                        savefile->settings.zoom = min;

                    sfx(SFX_MENUMOVE);
                }
            }

            #if defined(PC) || defined(WEB) || defined(PORTMASTER) || defined(SWITCH)
            refreshWindowSize();
            #endif
        }

        std::string getCurrentOption() override {
            if(savefile->settings.zoom > -1){
                return "x"+std::to_string((savefile->settings.zoom/10)+1)+"."+std::to_string(savefile->settings.zoom%10);
            }else{
                return "AUTO";
            }
        }
};

class WindowOptionScene : public OptionListScene{
    public:
    std::string name(){return "Video";};
        std::list<Element*> getElementList(){
            return {
                new IntegerScaleElement(),
                new ScaleElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};
