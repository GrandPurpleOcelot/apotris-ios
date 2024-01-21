#include "def.h"
#include "flashSaves.h"
#include "sprite1tiles_bin.h"
#include "sprites.h"
#include <string>
#include "logging.h"
#include <algorithm>
#include <cstring>

class SaveChange{
public:
    int saveId;
    int location;
    int size;

    SaveChange(int _id, int _location, int _size){
        saveId = _id;
        location = _location;
        size = _size;
    }
};

void setDefaultKeys();
void resetSkins(Save * save);
void setDefaults(Save * save, int depth);
void setDefaultGraphics(Save * save, int depth);
void applySaveChanges(u8* newSave, u8* oldSave,  int newSize);
void convertScores(Save *save);

const std::list<SaveChange> saveChanges = {
    SaveChange(0x50,132,20 * 4),//add zone + buffer to keys
    SaveChange(0x50,3120,4096),// add zone + buffer to scoreboards
};

void loadSave() {
    if(savefile != nullptr)
        delete savefile;

    savefile = new Save();
    loadSavefile();

    const int pmSettings = 540;
    const int pmSave = 3876;

    if (savefile->newGame == 0x50) {
        setDefaults(savefile,6);
    }else if (savefile->newGame == 0x4f) {
    }else if (savefile->newGame == 0x4e) {
        setDefaults(savefile,4);
    }else if (savefile->newGame == 0x4d) {
        Save* temp = new Save();
        const int oldSize = sizeof(Test3);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[pmSettings+ sizeof(u8)], &sf[oldSize], (pmSave - oldSize) / 2);

        setDefaults(temp,3);

        memcpy32(savefile, temp, pmSave / 4);

        delete temp;
    }else if (savefile->newGame == 0x4c) {
        Save* temp = new Save();
        const int oldSize = sizeof(Test2) + sizeof(u8);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[pmSettings + sizeof(u8)], &sf[oldSize], (pmSave - oldSize) / 2);

        setDefaults(temp,2);

        memcpy32(savefile, temp, pmSave / 4);

        setDefaultKeys();

        delete temp;
    }else if (savefile->newGame == 0x4b) {
        Save* temp = new Save();
        const int oldSize = sizeof(Test) + sizeof(u8);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[pmSettings + sizeof(u8)], &sf[oldSize], (pmSave - oldSize) / 2);

        setDefaults(temp,1);

        setDefaultKeys();

        memcpy32(savefile, temp, pmSave / 4);

        delete temp;

    } else if (savefile->newGame != SAVE_TAG) {
        savefile = new Save();

        setDefaults(savefile,0);

        setDefaultKeys();

        resetSkins(savefile);

        savefile->newGame = SAVE_TAG;
    }

    //fix invalid values, since I messed up conversion at some point
    if ((savefile->settings.lightMode != 0) && (savefile->settings.lightMode != 1))
        savefile->settings.lightMode = false;

    if ((savefile->settings.rumble < 0) || (savefile->settings.rumble > 4))
        savefile->settings.rumble = 0;

    if(savefile->settings.irs < 0 || savefile->settings.irs > 1)
        savefile->settings.irs = 1;
    if(savefile->settings.ihs < 0 || savefile->settings.ihs > 1)
        savefile->settings.ihs = 1;
    if(savefile->settings.initialType < 0 || savefile->settings.initialType > 1)
        savefile->settings.initialType = 1;

    if(savefile->newGame != SAVE_TAG){
        Save* temp = new Save();
        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        applySaveChanges(tmp, sf, sizeof(Save));

        memcpy32(savefile, temp, sizeof(Save) / 4);

        delete temp;
        savefile->newGame = SAVE_TAG;
    }

    u8* dump = (u8*)savefile;
    int sum = 0;
    for (int i = 0; i < (int)sizeof(Save); i++)
        sum += dump[i];
    sqran(sum);

    savefile->seed = qran();
    saveSavefile();
}

Keys getDefaultGameKeys(){
    Keys k {
        .moveLeft = KEY_LEFT,
        .moveRight = KEY_RIGHT,
        .rotateCCW = KEY_B,
        .rotateCW = KEY_A,
        .rotate180 = UNBOUND,
        .softDrop = KEY_DOWN,
        .hardDrop = KEY_UP,
        .hold = KEY_R,
        .zone = KEY_SELECT,
    };

    #ifdef GBA
    k.hold |= KEY_L;
    #endif

    return k;
}

MenuKeys getDefaultMenuKeys(){
    MenuKeys m{
        .up = KEY_UP,
        .down = KEY_DOWN,
        .left = KEY_LEFT,
        .right = KEY_RIGHT,
        .confirm = KEY_A,
        .cancel = KEY_B,
        .pause = KEY_START,
        .reset = UNBOUND,
        .special1 = KEY_L,
        .special2 = KEY_R,
        .special3 = KEY_SELECT,
    };

    #ifdef GBA
    m.reset = 0;
    #endif

    return m;
}

void setDefaultKeys(){
    savefile->settings.keys = getDefaultGameKeys();

    savefile->settings.menuKeys = getDefaultMenuKeys();
}

void resetSkins(Save *save){
    for(int i = 0; i < MAX_CUSTOM_SKINS; i++){
        memcpy16(&save->customSkins[i].board,sprite1tiles_bin,sprite1tiles_bin_size/2);
        memcpy16(&save->customSkins[i].smallBoard,mini[0],sprite1tiles_bin_size/2);
        save->customSkins[i].changed = false;
    }
}

void setDefaults(Save *save, int depth){

    if(depth < 1){
        save->settings.announcer = true;
        save->settings.finesse = false;
        save->settings.das = 11;
        save->settings.arr = 2;
        save->settings.sfr = 2;
        save->settings.dropProtection = true;
        save->settings.volume = 10;

        for (int i = 0; i < 8; i++)
            savefile->latestName[i] = ' ';
        savefile->latestName[8] = '\0';

        memset32(savefile->boards.marathon, 0, sizeof(EntryBoard) * 4 / 4);
        memset32(savefile->boards.sprint, 0, sizeof(EntryBoard) * 3 / 4);
        memset32(savefile->boards.dig, 0, sizeof(EntryBoard) * 3 / 4);
    }

    if(depth < 2){
        for (int i = 0; i < 10; i++)
            save->settings.songList[i] = true;

        memset32(savefile->boards.ultra, 0, sizeof(EntryBoard) * 3 / 4);
    }

    if(depth < 3){
        save->settings.maxQueue = 5;
        save->settings.sfxVolume = 10;
        save->settings.directionalDas = true;
        save->settings.noDiagonals = false;
        save->settings.cycleSongs = true;
        save->settings.dropProtectionFrames = 8;
        save->settings.abHold = true;
        save->settings.resetHoldType = false;

        memset32(savefile->boards.blitz, 0, sizeof(EntryBoard) * 2 / 4);
        memset32(&savefile->boards.combo, 0, sizeof(EntryBoard) * 1 / 4);
        memset32(savefile->boards.survival, 0, sizeof(EntryBoard) * 3 / 4);
    }

    if(depth < 4){
        save->settings.rumble = 0;
    }

    if(depth < 5){
        memset32(savefile->boards.sprintAttack, 0, sizeof(EntryBoard) * 3 / 4);
        memset32(savefile->boards.digEfficiency, 0, sizeof(EntryBoard) * 3 / 4);
        memset32(savefile->boards.classic, 0, sizeof(EntryBoard) * 2 / 4);

        save->stats.timePlayed = 0;
        save->stats.gamesStarted = 0;
        save->stats.gamesCompleted = 0;
        save->stats.gamesLost = 0;

        resetSkins(save);
    }

    if(depth < 6){
        memset32(savefile->boards.master, 0, sizeof(EntryBoard) * 2 / 4);
        memset32(savefile->boards.zone, 0, sizeof(EntryBoard) * 4 / 4);

        save->settings.diagonalType = save->settings.noDiagonals;
        save->settings.delaySoftDrop = false;
        save->settings.customDas = false;
        save->settings.irs = true;
        save->settings.ihs = true;
        save->settings.initialType = 1;
        save->settings.resetHoldToggle = 0;

        //check if select is already bound - if not, bind it to zone activation
        int* keys = (int*) &save->settings.keys;

        bool found = false;

        std::list<int> foundKeys;
        for(int i = 0; i < 9; i++){
            foundKeys.clear();

            int k = keys[i];
            int counter = 0;
            do{
                if(k & (1 << counter)){
                    foundKeys.push_back(1<<counter);
                    k-= 1 << counter;
                }

                counter++;
            }while(k != 0);

            if(std::find(foundKeys.begin(), foundKeys.end(),(int) KEY_SELECT) != foundKeys.end()){
                found = true;
                break;
            }
        }

        if(!found)
            save->settings.keys.zone = KEY_SELECT;
        else
            save->settings.keys.zone = 0;
    }

    if(depth < 7){
        if(depth)
            convertScores(savefile);

        save->settings.big = false;
        save->settings.pro = false;
        save->settings.goalLine = 2;
        save->settings.rotationSystem = BlockEngine::SRS;
        save->settings.randomizer = BlockEngine::BAG_7;
        save->settings.zoom = -1;
        save->settings.integerScale = true;
        save->settings.peek = true;
        save->settings.moveSfx = 2;
        save->settings.pauseCountdown = 1;
        save->settings.clearText = 2;
        save->settings.selectedProfile = 0;
        save->settings.journey = false;
        save->settings.autosave = 1;

        save->stats.maxLevel = 0;
        save->stats.totalLines = 0;
        save->stats.gameStats = BlockEngine::Stats();

        MenuKeys m{
            .up = KEY_UP,
            .down = KEY_DOWN,
            .left = KEY_LEFT,
            .right = KEY_RIGHT,
            .confirm = KEY_A,
            .cancel = KEY_B,
            .pause = KEY_START,
            .reset = UNBOUND,
            .special1 = KEY_L,
            .special2 = KEY_R,
            .special3 = KEY_SELECT,
        };

        #ifdef GBA
        m.reset = 0;
        #endif

        save->settings.menuKeys = m;

        strncpy(savefile->endTag, "SAVE_END", 9);
    }

    setDefaultGraphics(save, depth);

    for(int i = 0; i < 5; i++){
        saveToProfile(&save->settings.profiles[i],&save->settings);
    }
}

bool compareVersion(const SaveChange & first,const SaveChange & second){
    return first.saveId < second.saveId;
}

bool compareLocation(const SaveChange & first,const SaveChange & second){
    return first.location < second.location;
}

void applySaveChanges(u8* newSave, u8* oldSave, int newSize){

    const int migSize = 3876;
    const int version = savefile->newGame;

    int diff = newSize - migSize;
    int oldSize = migSize;

    std::list<SaveChange> sorted = saveChanges;

    //sort by descending location addresses
    sorted.sort(compareLocation);
    sorted.reverse();

    //sort by ascending save id
    sorted.sort(compareVersion);

    int applyingVersion = 0;
    int extraSize = 0;

    bool changed = false;
    int sum = 0;
    SaveChange previous = SaveChange(0,0,0);
    for(auto const & change : sorted){
        if(change.saveId <= version)
            continue;

        changed = true;

        if(change.saveId != applyingVersion){
            applyingVersion = change.saveId;
            oldSize += extraSize;
            extraSize = 0;
        }

        if(previous.saveId != 0 && previous.saveId != change.saveId){
            memcpy16(newSave,oldSave,previous.location / 2);

            memcpy16(oldSave,newSave,newSize/2);

            previous.location = 0;
        }

        int count = 0;
        if(previous.location == 0)
            count = oldSize-change.location;
        else
            count = previous.location - (change.location);

        memcpy16(&newSave[change.location + diff - sum],&oldSave[change.location], (count)/2);

        previous = change;
        sum += previous.size;
        extraSize += change.size;
    }

    if(previous.saveId != 0){
        memcpy16(newSave,oldSave,previous.location / 2);
    }

    if(!changed){
        memcpy32(newSave, oldSave, sizeof(Save) / 4);
    }
}

void setDefaultGraphics(Save *save, int depth){
    if(depth < 1){
        save->settings.floatText = true;
        save->settings.shake = true;
        save->settings.effects = true;
    }

    if(depth < 2){
        save->settings.edges = true;
        save->settings.backgroundGrid = 6;
        save->settings.skin = 12;
        save->settings.palette = 1;
        save->settings.shadow = 5;
        save->settings.lightMode = false;
    }

    if(depth < 3){
        save->settings.colors = 1;
        save->settings.clearEffect = 2;

        if(save->settings.shake){
            save->settings.shakeAmount = 2;
        }else{
            save->settings.shake = true;
            save->settings.shakeAmount = 0;
        }
    }

    if(depth < 4){
        save->settings.placeEffect = true;
        // save->settings.backgroundGradient = 0x6c525141;
        save->settings.backgroundGradient = 0x2dd827bf;
    }

    if(depth < 7){
        save->settings.frameBackground = 1;
        save->settings.backgroundType = 1;
        save->settings.aspectRatio = 0;
        save->settings.clearText = 2;

        #if defined(GBA)
        save->settings.screenShakeType = 0;
        #else
        save->settings.screenShakeType = 1;
        #endif

        save->settings.clearDirection = 0;
    }
}

void convertScores(Save *save){
    OldScoreboards *oldBoards = (OldScoreboards *) &save->boards;

    ModeBoards newBoards;

    memcpy32(&newBoards.marathon, oldBoards->marathon, sizeof(EntryBoard) * 4 / 4);
    memcpy32(&newBoards.sprint, &oldBoards->sprint, sizeof(EntryBoard) * 3 / 4);
    memcpy32(&newBoards.dig, &oldBoards->dig, sizeof(EntryBoard) * 3 / 4);
    memcpy32(&newBoards.ultra, &oldBoards->ultra, sizeof(EntryBoard) * 3 / 4);
    memcpy32(&newBoards.blitz, &oldBoards->blitz, sizeof(EntryBoard) * 2 / 4);
    memcpy32(&newBoards.combo, &oldBoards->combo, sizeof(EntryBoard) * 1 / 4);
    memcpy32(&newBoards.survival, &oldBoards->survival, sizeof(EntryBoard) * 3 / 4);
    memcpy32(&newBoards.sprintAttack, &oldBoards->sprintAttack, sizeof(EntryBoard) * 3 / 4);
    memcpy32(&newBoards.digEfficiency, &oldBoards->digEfficiency, sizeof(EntryBoard) * 3 / 4);
    memcpy32(&newBoards.classic, &oldBoards->classic, sizeof(EntryBoard) * 2 / 4);

    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 5; j++){
            newBoards.master[i].entries[j].grade = oldBoards->master[i].grade[j];
            newBoards.master[i].entries[j].value = oldBoards->master[i].times[j].frames;
            strncpy(newBoards.master[i].entries[j].name, oldBoards->master[i].times[j].name, 9);
        }
    }

    memcpy32(&newBoards.zone, &oldBoards->zone, sizeof(EntryBoard) * 4 / 4);

    memcpy32(&save->boards, &newBoards, sizeof(ModeBoards) / 4);
}

void profileToSave(GraphicsOptions *profile, Settings *settings){
    settings->clearText = profile->clearText;
    settings->shake = profile->shake;
    settings->effects = profile->effects;
    settings->edges = profile->edges;
    settings->backgroundGrid = profile->backgroundGrid;
    settings->skin = profile->skin;
    settings->palette = profile->palette;
    settings->shadow = profile->shadow;
    settings->lightMode = profile->lightMode;
    settings->colors = profile->colors;
    settings->clearEffect = profile->clearEffect;
    settings->shakeAmount = profile->shakeAmount;
    settings->backgroundGradient = profile->backgroundGradient;
    settings->placeEffect = profile->placeEffect;
    settings->frameBackground = profile->frameBackground;
    settings->backgroundType = profile->backgroundType;
    settings->aspectRatio = profile->aspectRatio;
    settings->screenShakeType = profile->screenShakeType;
    settings->clearDirection = profile->clearDirection;
}

void saveToProfile(GraphicsOptions *profile, Settings *settings){
    profile->clearText = settings->clearText;
    profile->shake = settings->shake;
    profile->effects = settings->effects;
    profile->edges = settings->edges;
    profile->backgroundGrid = settings->backgroundGrid;
    profile->skin = settings->skin;
    profile->palette = settings->palette;
    profile->shadow = settings->shadow;
    profile->lightMode = settings->lightMode;
    profile->colors = settings->colors;
    profile->clearEffect = settings->clearEffect;
    profile->shakeAmount = settings->shakeAmount;
    profile->backgroundGradient = settings->backgroundGradient;
    profile->placeEffect = settings->placeEffect;
    profile->frameBackground = settings->frameBackground;
    profile->backgroundType = settings->backgroundType;
    profile->aspectRatio = settings->aspectRatio;
    profile->screenShakeType = settings->screenShakeType;
    profile->clearDirection = settings->clearDirection;
}

void autosave(){
    if(!savefile->settings.autosave || multiplayer)
        return;

    if(framesSinceLastSave < savefile->settings.autosave * 5 * 3600)
        return;

    saveSavefile();
    framesSinceLastSave = 0;
}
