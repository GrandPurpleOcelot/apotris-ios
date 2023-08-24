#pragma once

#include "scene.hpp"

class BigModeElement : public Element{
    public:
        std::string getLabel() override {
            return "Big Mode";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.big = !savefile->settings.big;
            bigMode = savefile->settings.big;
            initFallingBlocks();

            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.big)
                return "ON";
            else
                return "OFF";
        }
};

class ProModeElement : public Element{
    public:
        std::string getLabel() override {
            return "Pro Mode";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.pro = !savefile->settings.pro;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.pro)
                return "ON";
            else
                return "OFF";
        }
};

class GoalLineElement : public Element{
    public:
        std::string getLabel() override {
            return "Goal Line";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.goalLine > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.goalLine < 2)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.goalLine < 2){
                    savefile->settings.goalLine++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.goalLine > 0){
                    savefile->settings.goalLine--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.goalLine){
                case 1: return "DOTTED";
                case 2: return "SOLID";
                default : return "OFF";
            }
        }
};

class RotationSystemElement : public Element{
    public:
        std::string getLabel() override {
            return "Rotation System";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.rotationSystem > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.rotationSystem < BlockEngine::ARS)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.rotationSystem < BlockEngine::ARS){
                    savefile->settings.rotationSystem++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.rotationSystem > 0){
                    savefile->settings.rotationSystem--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.rotationSystem){
                case BlockEngine::NRS: return "NRS";
                case BlockEngine::ARS: return "ARS";
                default : return "SRS";
            }
        }
};

class GameplayOptionScene : public OptionListScene{
    public:
    std::string name(){return "Gameplay";};
        std::list<Element*> getElementList(){
            return {
                new ProModeElement(),
                new GoalLineElement(),
                new RotationSystemElement(),
                new BigModeElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};
