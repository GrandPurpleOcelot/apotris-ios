#pragma once

#include "scene.hpp"

class AutoSaveElement : public Element{
    public:
        std::string getLabel() override {
            return "Autosave";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.autosave > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.autosave < 2)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.autosave < 2){
                    savefile->settings.autosave++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.autosave > 0){
                    savefile->settings.autosave--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            if(savefile->settings.autosave)
                return std::to_string(savefile->settings.autosave * 5) + "MIN";
            else
                return "OFF";
        }
};

class SavingOptionScene : public OptionListScene{
    public:
        std::string name(){return "Saving";};

        std::list<Element*> getElementList(){
            return {
                new AutoSaveElement(),
            };
        };

    Scene* previousScene(){return new MainMenuScene();};
};
