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
            MenuKeys temp = menuKeys;
            setDefaultKeys();
            menuKeys = savefile->settings.menuKeys;
            savefile->settings.menuKeys = temp;

            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            return "_";
        }
};

class RebindElement : public Element{
    public:
        std::string label;
        int index = 0;
        int menu = 0;

        int * keys = nullptr;

        bool reading = false;

        int max = 0;

        std::string getLabel() override {
            return label;
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action() override{
            std::list<int> foundKeys;

            reading = true;

            bool exit = false;

            while(closed()){
                canDraw = 1;
                vsync();
                key_poll();

                if(exit)
                    break;

                // if(key_hit(savefile->settings.menuKeys.pause)){
                //     sfx(SFX_MENUCANCEL);
                //     exit = true;
                //     continue;
                // }

                #if defined(GBA)

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

                    for(int i = 0; i < max; i++){
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
                    exit = true;
                    continue;
                }

                #elif defined(PC)

                bool found = false;
                for(int i = 0; i < sf::Keyboard::KeyCount; i++){
                    if(key_hit(i)){
                        keys[index] = i;
                        sfx(SFX_MENUCONFIRM);
                        found = true;

                        for(int j = 0; j < max; j++){
                            if(j == index)
                                continue;

                            if(keys[j] == i)
                                keys[j] = KEY_FULL - 2;
                        }

                        break;
                    }
                }

                if(found){
                    exit = true;
                    continue;
                }

                #else
                u32 key = key_first();

                if(key != KEY_FULL - 1){
                    keys[index] = key;


                    for(int j = 0; j < max; j++){
                        if(j == index)
                            continue;

                        if(keys[j] == key)
                            keys[j] = KEY_FULL - 2;
                    }

                    sfx(SFX_MENUCONFIRM);
                    exit = true;
                    continue;
                }

                #endif
            }

            reading = false;
        }

        std::string getCurrentOption() override {
            if(reading){
                return "...";
            }else if(keys[index] < 0){
                return "";
            }

            return getStringFromKey(keys[index]);
        }

        void reset(){
            if(menu == 0){
                MenuKeys k = getDefaultMenuKeys();

                int * src = (int*) &k;
                keys[index] = src[index];
            }else if(menu == 1){
                Keys k = getDefaultGameKeys();

                int * src = (int*) &k;
                keys[index] = src[index];
            }

            for (int i = 0; i < max; i++) {
                if (i == index)
                    continue;

                if (keys[i] == keys[index])
                    keys[i] = KEY_FULL - 2;
            }

            sfx(SFX_MENUCONFIRM);
        }

        RebindElement(int * _keys, int _menu, int _index, std::string _label, int _max){
            keys = _keys;
            menu = _menu;
            index = _index;
            label = _label;
            max = _max;
        }
};

class ControlOptionScene : public OptionListScene{
    public:
    std::string name(){return "Controls";};

    std::list<Element*> getElementList(){
        std::list<Element*> list;

        #if defined(PC) || defined(WEB)
        list.push_back(new LabelElement("In-Game:"));
        #endif

        int counter = 0;
        for(auto it = controlOptions.begin(); it != controlOptions.end(); ++it)
            list.push_back(new RebindElement((int *) &savefile->settings.keys,1,counter++,*it,9));

        #if defined(GBA) || defined(SWITCH) || defined(MM) || defined(RGB)
        list.push_back(new ABHoldElement());
        list.push_back(new QuickResetElement());
        list.push_back(new RumbleElement());

        #elif defined(PC) || defined(WEB)

        list.push_back(new LabelElement("Menu:"));

        counter = 0;
        for(auto it = menuControlOptions.begin(); it != menuControlOptions.end(); ++it)
            list.push_back(new RebindElement((int*) &menuKeys,0,counter++,*it,11));

        #endif

        list.push_back(new ResetControlsElement());

        return list;
    };

    bool control();

    Scene* previousScene(){return new SettingsScene();};
};
