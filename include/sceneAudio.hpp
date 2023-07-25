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
            return "[" + text + "]";
        }

        void action(int dir) override{
            savefile->settings.cycleSongs = !savefile->settings.cycleSongs;
            sfx(SFX_MENUMOVE);
        }

        std::string getCurrentOption() override {
            return (savefile->settings.cycleSongs) ? "CYCLE" : "LOOP";
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

        void action() override{
            sfx(SFX_MENUCONFIRM);
        }

        std::string getCurrentOption() override {
            return "...";
        }
};

class AudioOptionScene : public OptionListScene{
    public:
    std::string name(){return "Audio";};
        std::list<Element*> getElementList(){
            return {
                new MusicVolumeElement(),
                new SFXVolumeElement(),
                new AnnouncerElement(),
                new PlaybackElement(),
                new SongListElement(),
            };
        };

    Scene* previousScene(){return new SettingsScene();};
};
