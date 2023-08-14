#pragma once
#include "platform.hpp"
#include "blockEngine.h"

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
    int backgroundType;
    int aspectRatio;

    int placeHolder[90];
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

class Save{
public:
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

    bool gameExists = false;
    BlockEngine::Options savedGameOptions;
    BlockEngine::Game savedGame;

}ALIGN(4);

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
