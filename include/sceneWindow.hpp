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

class FPSElement : public Element{
    public:
        std::string getLabel() override {
            return "Show FPS";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.showFPS = !savefile->settings.showFPS;

            if(!savefile->settings.showFPS)
                clearText();

            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.showFPS)
                return "ON";
            else
                return "OFF";
        }
};

#ifdef PC
class FullscreenElement : public Element{
    public:
        std::string getLabel() override {
            return "Fullscreen";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.fullscreen = !savefile->settings.fullscreen;
            sfx(SFX_MENUCONFIRM);

            setFullscreen(savefile->settings.fullscreen);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.fullscreen)
                return "ON";
            else
                return "OFF";
        }
};
#endif

class WindowOptionScene : public OptionListScene{
    public:
    std::string name(){return "Video";};
        std::list<Element*> getElementList(){
            std::list<Element*> list;

            #ifdef PC
            list.push_back(new FullscreenElement);
            #endif

            list.push_back(new IntegerScaleElement());
            list.push_back(new ScaleElement());
            list.push_back(new FPSElement());

            return list;
        };

    Scene* previousScene(){return new SettingsScene();};
};
