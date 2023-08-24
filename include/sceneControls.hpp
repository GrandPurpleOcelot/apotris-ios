#pragma once

#include "scene.hpp"

class ABHoldElement : public Element{
    public:
        std::string getLabel() override {
            return "A+B to Hold";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.abHold = !savefile->settings.abHold;
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            if(savefile->settings.abHold)
                return "ON";
            else
                return "OFF";
        }
};

class QuickResetElement : public Element{
    public:
        std::string getLabel() override {
            return "Quick Restart";
        }

        std::string getCursor(std::string text) override {
            std::string result;
            if(!savefile->settings.resetHoldToggle)
                result += "<";
            else
                result += " ";

            result += text;

            if(!savefile->settings.resetHoldType)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.resetHoldToggle){
                    savefile->settings.resetHoldToggle = 0;
                    sfx(SFX_MENUMOVE);
                }else if(!savefile->settings.resetHoldType){
                    savefile->settings.resetHoldType = 1;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(!savefile->settings.resetHoldType){
                    savefile->settings.resetHoldToggle = 1;
                    sfx(SFX_MENUMOVE);
                }else if(savefile->settings.resetHoldType){
                    savefile->settings.resetHoldType = 0;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            if(savefile->settings.resetHoldToggle)
                return "OFF";

            if(savefile->settings.resetHoldType)
                return "HOLD";

            return "PRESS";
        }
};

class RumbleElement : public Element{
    public:
        std::string getLabel() override {
            return "Rumble";
        }

        std::string getCursor(std::string text) override {
            std::string result;
            if(savefile->settings.rumble > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.rumble < 4)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.rumble < 4){
                    savefile->settings.rumble++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.rumble > 0){
                    savefile->settings.rumble--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            if(savefile->settings.rumble)
                return std::to_string(savefile->settings.rumble * 25) + "%";
            else
                return "OFF";
        }
};

class ResetControlsElement : public Element{
    public:
        std::string getLabel() override {
            return "Reset Controls";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override{
            setDefaultKeys();

            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            return "";
        }
};

class RebindElement : public Element{
    public:
        std::string label;
        int index = 0;

        bool reading = false;

        std::string getLabel() override {
            return label;
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override{
            int* keys = (int*) &savefile->settings.keys;
            std::list<int> foundKeys;

            reading = true;

            while(closed()){
                canDraw = 1;
                vsync();
                key_poll();

                if(key_hit(KEY_START)){
                    sfx(SFX_MENUCANCEL);
                    break;
                }

                #ifdef GBA

                u16 key = key_hit(KEY_FULL);

                if(key != 0){
                    std::list<int> currentKeys;

                    int temp = key;

                    int counter = 0;
                    do{
                        if(temp & (1 << counter)){
                            currentKeys.push_back(1<<counter);
                            temp-= 1 << counter;
                        }

                        counter++;
                    }while(temp != 0);

                    for(int i = 0; i < 9; i++){
                        foundKeys.clear();

                        int k = keys[i];
                        counter = 0;
                        do{
                            if(k & (1 << counter)){
                                foundKeys.push_back(1<<counter);
                                k-= 1 << counter;
                            }

                            counter++;
                        }while(k != 0);

                        for(auto const & cur : currentKeys){
                            if(std::find(foundKeys.begin(), foundKeys.end(),(int) cur) != foundKeys.end())
                                keys[i]-=cur;
                        }
                    }

                    keys[index] = key;
                    sfx(SFX_MENUCONFIRM);
                    break;
                }

                #endif

                #ifdef PC

                bool found = false;
                for(int i = 0; i < sf::Keyboard::KeyCount; i++){
                    if(key_hit(i)){
                        keys[index] = i;
                        found = true;
                        break;
                    }
                }

                if(found){
                    sfx(SFX_MENUCONFIRM);
                    break;
                }

                #endif
            }

            reading = false;
        }

        std::string getCurrentOption() override {
            if(reading){
                return "...";
            }

            int *keys = (int *) &savefile->settings.keys;

            #ifdef GBA
            int key = keys[index];
            if((key & (key - 1)) == 0){
                return keyMap[keys[index]];
            }else{
                std::string result;

                int k = key;

                int counter = 0;
                do{
                    if(k & (1 << counter)){
                        result+= keyMap[1<<counter] + " ";
                        k-= 1 << counter;
                    }

                    counter++;
                }while(k != 0);

                //remove trailing space
                result.pop_back();

                return result;
            }
            #endif

            #ifdef PC
            return keyToString[keys[index]];
            #endif
        }

        RebindElement(int _index, std::string _label){
            index = _index;
            label = _label;
        }
};

class ControlOptionScene : public OptionListScene{
    public:
    std::string name(){return "Controls";};

    std::list<Element*> getElementList(){
        std::list<Element*> list;

        auto it = controlOptions.begin();

        int counter = 0;
        while(it != controlOptions.end()){
            list.push_back(new RebindElement(counter++,*it));

            it++;
        }

        #ifdef GBA
        list.push_back(new ABHoldElement());
        list.push_back(new QuickResetElement());
        list.push_back(new RumbleElement());
        #endif

        list.push_back(new ResetControlsElement());

        return list;
    };

    Scene* previousScene(){return new SettingsScene();};
};
