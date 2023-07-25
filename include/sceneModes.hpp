#pragma once

#include "scene.hpp"

class PlayOptionScene : public OptionListScene{
    public:

    int subMode = 0;
    int goal = 0;
    int level = 0;

    virtual Tetris::Modes getMode() = 0;

    bool control();

    virtual void start();

    Scene* previousScene(){return new ModeListScene();};
};

class LevelSelectorElement : public Element{
    public:
        int value = 1;

        int min = 0;
        int max = 1;

        int getValue() override{
            return value;
        }

        std::string getLabel() override {
            return "Level";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 1)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < 20)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(value < 20){
                    value++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(value > 1){
                    value--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            return std::to_string(value);
        }

        LevelSelectorElement(int _min, int _max){
            min = _min;
            max = _max;
        }
};

class StringSelectorElement : public Element{
    public:
        int value = 0;
        int options = 1;

        std::list<std::string> optionList;
        std::string label;

        virtual int getValue() override {return value;};

        std::string getLabel() override {return label;};

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < options - 1)
                result += ">";

            return result;
        }

        void action(int dir) override {
            if(dir > 0){
                if(value < options - 1){
                    value++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(value > 0){
                    value--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            if(value > (int) optionList.size())
                return "";

            auto it = optionList.begin();
            std::advance(it,value);

            return *it;
        }

        StringSelectorElement(std::list<std::string> list){
            optionList = list;
            options = optionList.size();
        }

        StringSelectorElement(std::string newLabel, std::list<std::string> list){
            optionList = list;
            options = optionList.size();
            label = newLabel;
        }

};

class IntSelectorElement : public Element{
    public:
        int value = 0;
        int options = 1;

        std::list<int> optionList;
        std::string label;

        virtual int getValue() override {
            if(value > (int) optionList.size())
                return 0;

            auto it = optionList.begin();
            std::advance(it,value);

            return *it;
        };

        std::string getLabel() override {return label;};

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < options - 1)
                result += ">";

            return result;
        }

        void action(int dir) override {
            if(dir > 0){
                if(value < options - 1){
                    value++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(value > 0){
                    value--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            if(value > (int) optionList.size())
                return "";

            auto it = optionList.begin();
            std::advance(it,value);

            return std::to_string(*it);
        }

        IntSelectorElement(std::list<int> list){
            optionList = list;
            options = optionList.size();
        }

        IntSelectorElement(std::string newLabel, std::list<int> list){
            optionList = list;
            options = optionList.size();
            label = newLabel;
        }

        IntSelectorElement(std::string newLabel, std::list<int> list, int _value){
            optionList = list;
            options = optionList.size();
            label = newLabel;
            value = _value;
        }
};

class TupleSelectorElement : public Element{
    public:
        int value = 0;
        int options = 1;

        std::list<std::tuple<int,std::string>> optionList;
        std::string label;

        std::string getLabel() override {return label;};

        int getValue() override {
            if(value > (int) optionList.size())
                return value;

            auto it = optionList.begin();
            std::advance(it,value);

            return std::get<0>(*it);
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(value > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < options - 1)
                result += ">";

            return result;
        }

        void action(int dir) override {
            if(dir > 0){
                if(value < options - 1){
                    value++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(value > 0){
                    value--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            if(value > (int) optionList.size())
                return "";

            auto it = optionList.begin();
            std::advance(it,value);

            return std::get<1>(*it);
        }

        TupleSelectorElement(std::list<std::tuple<int,std::string>> list){
            optionList = list;
            options = optionList.size();
        }

        TupleSelectorElement(std::string newLabel, std::list<std::tuple<int,std::string>> list){
            optionList = list;
            options = optionList.size();
            label = newLabel;
        }

};

class MarathonScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::MARATHON;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Zone"}),
            new LevelSelectorElement(1,20),
            new TupleSelectorElement("Lines",{{150,"150"},{200,"200"},{300,"300"},{0,"Endless"},}),
        };
    };

    std::string name(){return "Marathon";};
};

class SprintScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::SPRINT;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Attack"}),
            new IntSelectorElement("Lines",{20,40,100},1),
        };
    };

    std::string name(){return "Sprint";};
};

class DigScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::DIG;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Efficiency"}),
            new IntSelectorElement("Lines",{10,20,100}),
        };
    };

    std::string name(){return "Dig";};
};

class UltraScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::ULTRA;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Minutes",{{3 * 3600,"3"},{5 * 3600,"5"},{10* 3600, "10"}}),
        };
    };

    std::string name(){return "Ultra";};
};

class BlitzScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::BLITZ;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    std::string name(){return "Blitz";};

    BlitzScene(){
        goal = 2 * 3600;
    }
};

class ComboScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::COMBO;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    std::string name(){return "Combo";};
};

class SurvivalScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::SURVIVAL;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Difficulty",{{1,"EASY"},{2,"MEDIUM"},{3,"HARD"}}),
        };
    };

    std::string name(){return "Survival";};
};

class ClassicScene : public PlayOptionScene{
    public:

    Tetris::Modes getMode(){return Tetris::CLASSIC;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"A-TYPE","B-TYPE"}),
            new LevelSelectorElement(0,19),
        };
    };

    std::string name(){return "Classic";};
};
