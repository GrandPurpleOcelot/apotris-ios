#pragma once

#include "scene.hpp"

class MusicVolumeElement : public Element{
    public:
        std::string getLabel() override {
            return "Music";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.volume > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.volume < 10)
                result += ">";

            return result;
        }

        void action(int dir) override{
            int previous = savefile->settings.volume;

            if(dir > 0){
                if(savefile->settings.volume < 10){
                    savefile->settings.volume++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.volume > 0){
                    savefile->settings.volume--;
                    sfx(SFX_MENUMOVE);
                }
            }

            if(previous != savefile->settings.volume && previous == 0){
                playSongRandom(currentMenu);
            }

            setMusicVolume(512 * ((float)savefile->settings.volume / 10));
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.volume);
        }
};

class SFXVolumeElement : public Element{
    public:
        std::string getLabel() override {
            return "SFX";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.sfxVolume > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.sfxVolume < 10)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.sfxVolume < 10){
                    savefile->settings.sfxVolume++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.sfxVolume > 0){
                    savefile->settings.sfxVolume--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->settings.sfxVolume);
        }
};

class AnnouncerElement : public Element{
    public:
        std::string getLabel() override {
            return "Announcer";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.announcer = !savefile->settings.announcer;
            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            return (savefile->settings.announcer) ? "ON" : "OFF";
        }
};

class PlaybackElement : public Element{
    public:
        std::string getLabel() override {
            return "Playback";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.cycleSongs > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.cycleSongs < 2)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.cycleSongs < 2){
                    savefile->settings.cycleSongs++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.cycleSongs > 0){
                    savefile->settings.cycleSongs--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.cycleSongs){
                case 0: return "LOOP";
                case 1: return "CYCLE";
                case 2: return "SHUFFLE";
                default: return "";
            }
        }
};

class MoveSfxElement : public Element{
    public:
        std::string getLabel() override {
            return "Piece Move SFX";
        }

        std::string getCursor(std::string text) override {
            std::string result;

            if(savefile->settings.moveSfx > 0)
                result += "<";
            else
                result += " ";

            result += text;

            if(savefile->settings.moveSfx < 2)
                result += ">";

            return result;
        }

        void action(int dir) override{
            if(dir > 0){
                if(savefile->settings.moveSfx < 2){
                    savefile->settings.moveSfx++;
                    sfx(SFX_MENUMOVE);
                }
            }else{
                if(savefile->settings.moveSfx > 0){
                    savefile->settings.moveSfx--;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        std::string getCurrentOption() override {
            switch(savefile->settings.moveSfx){
                case 0: return "OFF";
                case 1: return "INITIAL";
                case 2: return "ALL";
                default: return "";
            }
        }
};

class SongElement : public Element{
    public:
        int menu = 0;
        int index = 0;

        std::string getLabel() override {
            return "Track " + std::to_string(index+1);
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        std::string getCurrentOption() override {
            if(!getSongState(menu, index))
                return "ON";
            else
                return "OFF";
        }

        void action(int dir) override {
            toggleSong(menu, index, !getSongState(menu, index));
            sfx(SFX_MENUMOVE);
        }

        void select();

        SongElement(int m, int i){
            menu = m;
            index = i;
        }
};

class SongListElement : public Element{
    public:
        std::string getLabel() override {
            return "Song List";
        }

        std::string getCursor(std::string text) override {
            return "[" + text + "]";
        }

        bool action() override;

        std::string getCurrentOption() override {
            return "...";
        }
};

class AudioOptionScene : public OptionListScene{
    public:
        std::string name() { return "Audio"; };

        std::list<Element *> getElementList() {
            return {
                new MusicVolumeElement(),
                new SFXVolumeElement(),
                new AnnouncerElement(),
                new PlaybackElement(),
                new MoveSfxElement(),
                new SongListElement(),
            };
        };

        Scene *previousScene() { return new SettingsScene(); };
};

class SongListScene : public OptionListScene{
    public:
        int previous = 0;
        std::string name(){return "Song List";};

        std::list<Element*> getElementList() {
            std::list<Element*> list;

            list.push_back(new LabelElement("Menu:"));

            for(int i = 0; i < (int) songs.menu.size(); i++){
                list.push_back(new SongElement(0,i));
            }

            list.push_back(new LabelElement("In-Game:"));

                for(int i = 0; i < (int) songs.game.size(); i++){
                list.push_back(new SongElement(1,i));
            }

            return list;
        };

        void deinit(){
            savefile->settings.cycleSongs = previous;
            stopSong();
            playSongRandom(0);
        }

        void init();
        bool control();

        Scene *previousScene() { return new AudioOptionScene();};

        SongListScene(){
            previous = savefile->settings.cycleSongs;

            savefile->settings.cycleSongs = 0;//LOOP
        }

        ~SongListScene(){
            deinit();
        }
};
