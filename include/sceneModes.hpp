#pragma once

#include "scene.hpp"

class PlayOptionScene : public OptionListScene{
    public:

    int subMode = 0;
    int goal = 0;
    int level = 0;

    bool onScore = false;
    bool onStart = false;
    int movingScore = 0;

    int movingScoreTimer = 0;
    int movingScoreTimerMax = 8;

    OBJ_ATTR * proSprites[5];

    WordSprite startSprite = WordSprite(0, 20, 150);

    std::string currentElement = "";

    virtual bool getIfTime(){return false;};
    virtual bool getIfGrade(){return false;};
    virtual BlockEngine::Modes getMode() = 0;
    virtual void start();
    virtual EntryBoard* getBoard(int submode, int goal){return nullptr;};

    void setOptions();
    void resetScoreboard();

    void init();
    bool control();
    void draw();
    void update();

    Scene* previousScene(){return new ModeListScene();};
};

class LevelSelectorElement : public Element{
    public:
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

            if(value > min)
                result += "<";
            else
                result += " ";

            result += text;

            if(value < max)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(value < max){
                    value++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(value > min){
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

            value = min;
        }
};

class StringSelectorElement : public Element{
    public:
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

    BlockEngine::Modes getMode(){return BlockEngine::MARATHON;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Zone"}),
            new LevelSelectorElement(1,20),
            new TupleSelectorElement("Lines",{{150,"150"},{200,"200"},{300,"300"},{0,"Endless"},}),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        if(submode == 0){
           return &savefile->boards.marathon[goal];

        }else{
           return &savefile->boards.zone[goal];

        }
    }

    std::string name(){return "Marathon";};
};

class SprintScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::SPRINT;};
    bool getIfTime(){return true;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Attack"}),
            new IntSelectorElement("Lines",{20,40,100},1),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        if(submode == 0){
            return &savefile->boards.sprint[goal];
        }else{
            return &savefile->boards.sprintAttack[goal];
        }
    }

    std::string name(){return "Sprint";};
};

class DigScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::DIG;};

    bool getIfTime(){return !subMode;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"Normal","Efficiency"}),
            new IntSelectorElement("Lines",{10,20,100}),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        if(submode == 0){
           return &savefile->boards.dig[goal];

        }else{
           return &savefile->boards.digEfficiency[goal];

        }
    }

    std::string name(){return "Dig";};
};

class UltraScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::ULTRA;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Minutes",{{3 * 3600,"3"},{5 * 3600,"5"},{10* 3600, "10"}}),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        return &savefile->boards.ultra[goal];
    }

    std::string name(){return "Ultra";};
};

class BlitzScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::BLITZ;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        return &savefile->boards.blitz[0];
    }

    std::string name(){return "Blitz";};

    BlitzScene(){
        goal = 2 * 3600;
    }
};

class ComboScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::COMBO;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        return &savefile->boards.combo;
    }

    std::string name(){return "Combo";};
};

class SurvivalScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::SURVIVAL;};

    bool getIfTime(){return true;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Difficulty",{{1,"EASY"},{2,"MEDIUM"},{3,"HARD"}}),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        return &savefile->boards.survival[goal];
    }

    std::string name(){return "Survival";};
};

class ClassicScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::CLASSIC;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Type",{"A-TYPE","B-TYPE"}),
            new LevelSelectorElement(0,19),
        };
    };

    void update();

    EntryBoard* getBoard(int submode, int goal){
        return &savefile->boards.classic[submode];
    }

    std::string name(){return "Classic";};
};

class MasterScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::MASTER;};

    bool getIfTime(){return true;};
    bool getIfGrade(){return true;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Rules",{"Normal","Classic"}),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        return &savefile->boards.master[submode];
    }

    std::string name(){return "Master";};
};

class DeathScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::DEATH;};

    bool getIfTime(){return true;};
    bool getIfGrade(){return true;};

    std::list<Element*> getElementList(){
        return {
            new StringSelectorElement("Rules",{"Normal","Classic"}),
        };
    };

    EntryBoard* getBoard(int submode, int goal){
        return nullptr;
        // return &savefile->boards.master[submode];
    }

    std::string name(){return "Death";};
};

class ZenScoreElement : public Element{
    std::string getLabel() override {
        return "Score";
    }

    std::string getCurrentOption() override {
        return std::to_string(savefile->boards.zen);
    }

    std::string getCursor(std::string text) override {
        return text;
    }
};

class ZenScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::ZEN;};

    std::list<Element*> getElementList(){
        return {
            new ZenScoreElement(),
        };
    };

    std::string name(){return "Zen";};
};

class TrainingScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::TRAINING;};

    std::list<Element*> getElementList(){
        return {
            new LevelSelectorElement(0,20),
            new StringSelectorElement("Finesse Training",{"OFF","ON"}),
        };
    };

    std::string name(){return "Training";};
};

class MultBattleScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::BATTLE;};

    std::list<Element*> getElementList(){
        return {
        };
    };

    void init();
    bool control();
    void update();
    void deinit();
    void draw();

    std::string name(){return "2P Battle";};
};

class CPUBattleScene : public PlayOptionScene{
    public:

    BlockEngine::Modes getMode(){return BlockEngine::BATTLE;};

    std::list<Element*> getElementList(){
        return {
            new TupleSelectorElement("Difficulty",{{1,"EASY"},{2,"MEDIUM"},{3,"HARD"},{4,"V.HARD"}}),
        };
    };

    std::string name(){return "CPU Battle";};
};
