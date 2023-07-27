#include "scene.hpp"

std::string getDescription(std::string element){

    if(element == "Marathon"){
        return "Try to get the best score as the game gets faster and faster";

    }else if(element == "Sprint"){
        return "Clear lines as fast as you can";

    }else if(element == "Dig"){
        return "Dig through lines of garbage";

    }else if(element == "Ultra"){
        return "Score as much as possible in a limited time";

    }else if(element == "Blitz"){
        return "Get the best score in a limited time while the game gets faster and faster";

    }else if(element == "Combo"){
        return "Aim for the most consecutive clears in a 4-wide well";

    }else if(element == "Survival"){
        return "Survive as long as possible while garbage rises from below";

    }else if(element == "Classic"){
        return "Old school stacking mechanics";

    }else if(element == "Master"){
        return "Become a Grandmaster";

    }else if(element == "2P Battle"){
        return "Battle your friends through the Link Cable";

    }else if(element == "CPU Battle"){
        return "Go against an AI opponent";

    }else if(element == "Training"){
        return "Try out new strategies or improve your playing";

    }

    return "";
}
