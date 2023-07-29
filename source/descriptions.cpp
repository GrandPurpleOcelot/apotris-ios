#include "scene.hpp"

std::string getDescription(std::string element){

// MODES ---------------------------------------------------------------------------------
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

// Graphics ---------------------------------------------------------------------------------

// AUDIO ---------------------------------------------------------------------------------
    }else if(element == "Announcer"){
        return "Announcer SFX when you clear lines";

    }else if(element == "Playback"){
        return "Cycle through songs or Loop the same song";

    }else if(element == "Song List"){
        return "Enable/Disable songs";

// CONTROL -------------------------------------------------------------------------------
    }else if(element == "A+B to Hold"){
        return "Hold when pressing A and B";

    }else if(element == "Quick Restart"){
        return "Press/Hold L and R to restart the game";

    }else if(element == "Rumble"){
        return "Needs compatible hardware";

    }else if(element == "Reset Controls"){
        return "Set bindings back to default";

// HANDLING ------------------------------------------------------------------------------
    }else if(element == "DAS"){
        return "Delay before auto repeat starts";

    }else if(element == "ARR"){
        return "How often auto repeat activates";

    }else if(element == "SDF"){
        return "Soft Drop Auto Repeat Rate";

    }else if(element == "Delay Soft Drop"){
        return "Whether DAS affects Soft Drop or not";

    }else if(element == "Drop Protection"){
        return "Disables back-to-back hard drops for the set frames";

    }else if(element == "Directional Delay"){
        return "Reset DAS on direction change";

    }else if(element == "Disable Diagonals"){
        return "STRICT ignores diagonals completely, SOFT registers the second direction when the first gets released";

    }else if(element == "Initial Hold"){
        return "Buffer hold during delays";

    }else if(element == "Initial Rotation"){
        return "Buffer rotations during delays";

    }else if(element == "Initial System"){
        return "TYPE A always buffers actions, TYPE B only buffers if you started holding the button during the delay";
    }

    return "";
}
