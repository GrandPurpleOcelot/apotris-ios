#pragma once

#include "platform.hpp"
#include "blockEngine.h"
#include "text.h"
#include "menu.h"

INLINE FIXED lerp(FIXED a, FIXED b, FIXED mix){
    return a + (((b-a)*mix)>>8);
}

INLINE int flipSign(int x, int y){
    return (((x > 0) - (x < 0)) ^ y) - y;
}

extern const u16 fontTiles[1552];
#define fontTilesLen 3104
extern const u16 font3x5[96];

typedef struct Highscore{
    char name[9];
    int score;
}ALIGN(4) Highscore;

typedef struct Scoreboard{
    Highscore highscores[5];
}ALIGN(4) Scoreboard;

typedef struct Time{
    char name[9];
    int frames;
}ALIGN(4) Time;

typedef struct Timeboard{
    Time times[5];
}ALIGN(4) Timeboard;

typedef struct Gradeboard{
    Time times[5];
    s8 grade[5];
}ALIGN(4) Gradeboard;

typedef struct Keys{
    int moveLeft;
    int moveRight;
    int rotateCCW;
    int rotateCW;
    int rotate180;
    int softDrop;
    int hardDrop;
    int hold;
    int zone;

    int placeHolder[19];
}ALIGN(4) Keys;

typedef struct QuickStart{
    int mode;
    int submode;
    int goal;
    int level;
    int big;
}ALIGN(4) QuickStart;

typedef struct Settings{

    bool announcer;
    bool finesse;
    bool floatText;
    bool shake;
    int effects;
    int volume;
    int das;
    int arr;
    int sfr;
    bool dropProtection;
    int backgroundGrid;
    bool edges;
    int skin;
    int palette;
    int shadow;
    bool lightMode;
    bool songList[10];
    int sfxVolume;
    bool directionalDas;
    int shakeAmount;
    bool noDiagonals;
    int maxQueue;
    int colors;
    bool cycleSongs;
    int dropProtectionFrames;
    bool abHold;
    Keys keys;
    int clearEffect;
    bool resetHoldType;
    bool placeEffect;
    bool resetHoldToggle;
    int rumble;
    int diagonalType;
    bool delaySoftDrop;
    int backgroundGradient;
    bool customDas;
    int ihs;
    int irs;
    int initialType;
    int frameBackground;

    int placeHolder[92];
}ALIGN(4) Settings;

typedef struct Test{
    bool t1[6];
    int t2[4];
}ALIGN(4) Test;

typedef struct Test2{
    bool t1[18];
    int t2[10];
}ALIGN(4) Test2;

typedef struct Test3{
    bool t1[14];//14
    int t2[30];//22
}ALIGN(4) Test3;

typedef struct TotalStats{
    int timePlayed;
    int gamesStarted;
    int gamesCompleted;
    int gamesLost;
}ALIGN(4) TotalStats;

typedef struct Skin{
    TILE board;
    TILE smallBoard;
    int previewStyle;
}ALIGN(4) Skin;

#define MAX_CUSTOM_SKINS 5

typedef struct SaveEntry{
    char name[9];
    bool pro;
    s16 grade;
    int value;
}ALIGN(4) SaveEntry;

typedef struct EntryBoard{
   SaveEntry entries[5];
}ALIGN(4) EntryBoard;

typedef struct OldScoreboards{
    Scoreboard marathon[4];
    Timeboard sprint[3];
    Timeboard dig[3];
    Scoreboard ultra[3];
    Scoreboard blitz[2];
    Scoreboard combo;
    Timeboard survival[3];
    Timeboard sprintAttack[3];
    Scoreboard digEfficiency[3];
    Scoreboard classic[2];
    Gradeboard master[2];
    Scoreboard zone[4];
}ALIGN(4) OldScoreboards;

typedef struct ModeBoards{
    EntryBoard marathon[4];
    EntryBoard sprint[3];
    EntryBoard dig[3];
    EntryBoard ultra[3];
    EntryBoard blitz[2];
    EntryBoard combo;
    EntryBoard survival[3];
    EntryBoard sprintAttack[3];
    EntryBoard digEfficiency[3];
    EntryBoard classic[2];
    EntryBoard master[2];
    EntryBoard zone[4];
}ALIGN(4) ModeBoards;

typedef struct Save{
    u8 newGame;

    Settings settings;
    int seed;
    char latestName[9];

    ModeBoards boards;

    int placeHolder[1000];

    TotalStats stats;

    int placeHolder2[100];

    Skin customSkins[MAX_CUSTOM_SKINS];

    QuickStart quickStart;
}ALIGN(4) Save;

typedef struct ReplayHeader{
    u8 tag;
    u32 length;
    u32 duration;
    u32 seed;

    BlockEngine::Options options;

    int placeholder[240];
}ALIGN(4) ReplayHeader;

typedef struct Replay{
    ReplayHeader header;
    uint32_t moves[4096];
}ALIGN(4) Replay;

extern Save *savefile;

#define glowDuration 12

class FloatText {
public:
    std::string text;
    int timer = 0;

    FloatText() {}
    FloatText(std::string _t) {
        text = _t;
    }
};

class Effect {
public:
    int timer = 0;
    int duration;
    int type;
    int x;
    int y;

    Effect() {}
    Effect(int _type) {
        type = _type;
        duration = glowDuration * 3;
    }
    Effect(int _type, int _x, int _y) {
        type = _type;
        duration = glowDuration * (3 / 2);
        x = _x;
        y = _y;
    }
};

class PlaceEffect{
public:

        int x;
        int y;
        int dx;
        int dy;
        int piece;
        int rotation;
        int rotating;

        int timer = 12;

        OBJ_ATTR* sprite;

        PlaceEffect(){}
        PlaceEffect(int _x, int _y, int _dx, int _dy, int _piece, int _rotation, int _rotating){
            x = (_x + 10) * 8 - 16 - 32;
            y = _y * 8 - 16;
            dx = _dx;
            dy = _dy;
            piece = _piece;
            rotation = _rotation;
            rotating = _rotating;
        }
};

#define TRAINING_MESSAGE_MAX 300
#define MAX_SKINS 14
#define MAX_SHADOWS 5
#define MAX_BACKGROUNDS 8
#define MAX_COLORS 7
#define MAX_CLEAR_EFFECTS 4
#define MAX_FRAME_BACKGROUNDS 3

#define MAX_MENU_SONGS 3
#define MAX_GAME_SONGS 6

#define GRADIENT_COLOR 0x71a6

#define SHOW_FINESSE 1
#define DIAGNOSE 1
#define SAVE_TAG 0x50
#define ENABLE_SHADERS 0

#define ENABLE_FLASH_SAVE 1

extern void sfx(int);
extern void gameLoop();
extern void playSong(int,int);
extern void playSongRandom(int);
extern void playNextSong();
extern void settingsText();
extern void songListMenu();
extern void graphicTest();
extern void windowSettings();
extern void audioSettings();
extern void showTitleSprites();
extern void setLightMode();
extern void setSkin();
extern void setDefaultKeys();
extern void setClearEffect();

extern void showBackground(int offset);
extern void showPawn();
extern void showShadow();
extern void showQueue();
extern void showHold();
extern void drawGrid();
extern void drawFrame(int layer);
extern void drawFrameBackgrounds();
extern void clearGlow();
extern void showClearText();
extern void hideMinos();

extern void reset();
extern void sleep();

extern void handleMultiplayer(int state);
extern void startMultiplayerGame(int);
extern void progressBar(int layer);

extern void screenShake();

extern void saveToSram();
extern void addToResults(int,int);

extern void drawEnemyBoard();
extern void handleBotGame();
extern void showPPS();
extern void setPalette();
extern void loadSave();
extern void startScreen();
extern void showPlaceEffect();
extern void checkSounds();
extern int getClassicPalette();
extern void maxModInit();
extern void drawUIFrame(int,int,int,int);
extern void buildMini(TILE *);
extern void showZoneMeter();
extern void resetZonePalette();
extern void showBestMove();
extern BlockEngine::Tuning getTuning();
extern void showFinesseCombo();
extern void setGradient(int);
extern void setDefaultGradient();
extern void gradient(int state);
extern std::string timeToString(int frames);
extern std::string timeToStringHours(int frames);
extern void setDefaultGraphics(Save * save, int depth);
extern void showCredits();
extern void setupCredits();
extern void refreshCredits();
extern void setPawnPalette(int dest, int n, int blend);

extern void startGame(BlockEngine::Options options, int seed);
extern void startGame(int seed);
extern void startGame();

extern void startBotGame(int seed);
extern int countBits(u16 n);
extern int countTransitions(u16 a);
extern void addStats();

extern void loadReplay();
extern void frameSnow(int layer);

extern void initFallingBlocks();

extern void gradientEditor();

extern bool settingsChanged();

extern void fallingBlocks();

extern void updateGrid();

extern std::string modeToString(BlockEngine::Modes mode);

extern BlockEngine::Game *game;
extern BlockEngine::Game *quickSave;
extern BlockEngine::Game *botGame;

extern u8 * blockSprite;

extern int shake;
extern int shakeMax;

extern bool onStates;

extern bool multiplayer;

extern bool paused;

extern bool restart;

extern std::list<Effect> effectList;
extern std::list<FloatText> floatingList;
extern std::list<PlaceEffect> placeEffectList;

extern s16 glow[20][10];

extern int nextSeed;

extern int push;
#define pushMax 4

extern bool canDraw;
extern int gameSeconds;

extern bool playAgain;

// extern bool resumeJourney;
// extern bool journeyLevelUp;
// extern bool journeySaveExists;
// extern BlockEngine::Game* journeySave;

extern int connected;
extern int multiplayerStartTimer;

extern int initialLevel;
extern int frameCounter;

extern OBJ_ATTR * titleSprites[2];
extern OBJ_ATTR * queueFrameSprites[3];

extern int enemyHeight;

extern BlockEngine::Bot *testBot;
extern u8 enemyBoard[20][10];

extern int mode;

extern int currentlyPlayingSong;
extern int currentMenu;

extern int previousOptionScreen;
extern bool goToOptions;

extern int rumbleTimer;
extern int rumbleMax;

extern bool rumbleInitialized;
extern bool bigMode;

extern int goalSelection;
extern int level;

extern TILE* customSkin;

extern bool proMode;

extern bool gradientEnabled;

extern int botThinkingSpeed;
extern int botSleepDuration;

extern int clearTimer;
extern int maxClearTimer;

extern BlockEngine::Options *previousGameOptions;

extern FIXED shakeBuff;
extern FIXED shakeVelocity;

extern std::list<BlockEngine::Timestamp*>::iterator replayIterator;
extern std::list<BlockEngine::Timestamp*> currentReplay;

extern BlockEngine::Game *quickSave;
extern bool saveExists;

#define shakeMax 10

extern bool replaying;

extern bool enableBot;
extern bool demo;

extern int current[22][12];
extern int previous[22][12];

extern WordSprite* wordSprites[MAX_WORD_SPRITES];

extern Settings previousSettings;

extern std::list<std::string> path;
extern std::string previousElement;

extern int gridUpdateTimer;
extern int gridUpdateTimerMax;
extern int damp;
extern int dampTimer;
extern int dampTimerMax;

class Function{
    private:

    public:
        void (BlockEngine::Game::*gameFunction)(int);
};
