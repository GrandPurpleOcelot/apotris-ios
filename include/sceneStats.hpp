#pragma once

#include "scene.hpp"

class PlayTimeElement : public Element{
    public:
        std::string getLabel() override {
            return "Play Time";
        }

        std::string getCursor(std::string text) override {
            return " " + text + " ";
        }

        std::string getCurrentOption() override {
            return timeToStringHours(savefile->stats.timePlayed);
        }
};

class GamesStartedElement : public Element{
    public:
        std::string getLabel() override {
            return "Games Started";
        }

        std::string getCursor(std::string text) override {
            return " " + text + " ";
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->stats.gamesStarted);
        }
};

class GamesCompletedElement : public Element{
    public:
        std::string getLabel() override {
            return "Games Completed";
        }

        std::string getCursor(std::string text) override {
            return " " + text + " ";
        }

        std::string getCurrentOption() override {
            return std::to_string(savefile->stats.gamesCompleted);
        }
};

class StatScene : public OptionListScene{
    public:
        std::string name(){return "Stats";};

        std::list<Element*> getElementList(){
            return {
                new PlayTimeElement(),
                new GamesStartedElement(),
                new GamesCompletedElement(),
            };
        };

    Scene* previousScene(){return new MainMenuScene();};
};
