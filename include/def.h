#pragma once

#include "version.h"

#include "blockEngine.hpp"
#include "menu.h"
#include "platform.hpp"
#include "save.h"
#include "text.h"
#include <list>
#include <memory>

#define glowDuration 12
#ifdef GBA
#include "LinkUniversal.hpp"
extern LinkUniversal* linkConnection;
extern bool disableHBlank;
#endif
extern int initialPlayerCount;

INLINE FIXED lerp(FIXED a, FIXED b, FIXED mix) {
    return a + (((b - a) * mix) >> 8);
}

INLINE int flipSign(int x, int y) { return (((x > 0) - (x < 0)) ^ y) - y; }

extern const u16 fontTiles[1552];
#define fontTilesLen 3104
extern const u16 font3x5[96];

class FloatText {
public:
    std::string text;
    int timer = 0;

    FloatText() {}
    FloatText(std::string _t) { text = _t; }
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

class PlaceEffect {
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

    PlaceEffect() {}
    PlaceEffect(int _x, int _y, int _dx, int _dy, int _piece, int _rotation,
                int _rotating) {
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
#define MAX_SKINS 15
#define MAX_SHADOWS 8
#define MAX_BACKGROUNDS 8
#define MAX_COLORS 7
#define MAX_CLEAR_EFFECTS 4
#define MAX_FRAME_BACKGROUNDS 3

#define MAX_MENU_SONGS 3
#define MAX_GAME_SONGS 6

#define GRADIENT_COLOR 0x71a6

#define SHOW_FINESSE 1
#define SAVE_TAG 0x52

extern void sfx(int);
extern void gameLoop();
extern void playSong(int, int);
extern void playSongRandom(int);
extern void playNextSong();
extern void songListMenu();
extern void showTitleSprites();
extern void setLightMode();
extern void setSkin();
extern void setSkin(int rotation);
extern void setDefaultKeys();
extern void setClearEffect();

extern void showBackground(int offset);
extern void showPawn();
extern void showShadow();
extern void showQueue(bool offsetSpriteIndex);
extern void showHold();
extern void drawGrid();
extern void drawFrame(int layer);
extern void drawFrameBackgrounds();
extern void clearGlow();
extern void showClearText();
extern void hideMinos();

extern void reset();

extern void handleMultiplayer(int state);
extern void startMultiplayerGame(int);
extern void acknowledgeGarbage(BlockEngine::Game*, BlockEngine::Garbage);
extern void progressBar(int layer);

extern void saveToSram();
extern void addToResults(int, int);

extern void handleBotGame();
extern void showPPS();
extern void showGoalLine();
extern void setPalette();
extern void loadSave();
extern void showPlaceEffect();
extern int getClassicPalette();
extern void buildMini(TILE*);
extern void showZoneMeter();
extern void resetZonePalette();
extern void showBestMove();
extern BlockEngine::Tuning getTuning();
extern void showFinesseCombo();
extern void setGradient(int);
extern void setDefaultGradient();
extern void gradient(int state);
extern std::string timeToString(int frames, bool small);
extern std::string timeToStringHours(int frames);
extern void setDefaultGraphics(Save* save, int depth);
extern void setRandomGraphics(Save* save);
extern void setPawnPalette(int dest, int n, int blend, bool flip);

extern void startGame(BlockEngine::Options options, int seed);
extern void startGame(int seed);
extern void startGame();

extern void startBotGame(int seed);

extern void loadReplay();
extern void frameSnow(int layer);

extern void initFallingBlocks();

extern void gradientEditor();

extern bool settingsChanged();

extern void fallingBlocks();

extern void updateGrid();

extern void updateFluid();

extern void addGameStats();

extern void setPreviousSettings(Settings settings);

extern std::string modeToString(BlockEngine::Modes mode);

extern std::string getStringFromKey(int key);

extern void checkPeek();
extern MenuKeys getDefaultMenuKeys();
extern Keys getDefaultGameKeys();
extern void setDefaultHandling(Save* save);
extern void setDefaultGameplay(Save* save);

extern bool getSongState(int group, int index);
extern void toggleSong(int group, int index, bool state);
extern void checkSongs();

extern int countClears(const u16* board, int lengthX, int lengthY, int startY);
extern IWRAM_CODE BlockEngine::Values evaluate(u16* board,
                                               const int* columnHeights,
                                               int lengthX, int lengthY,
                                               int startY);
extern int findBestDrop(int ii, int jj, BlockEngine::Game* game,
                        std::list<BlockEngine::Move>& moves,
                        BlockEngine::Weights weights);

extern BlockEngine::Game* game;
extern BlockEngine::Game* quickSave;
extern BlockEngine::Game* botGame;

extern u8* blockSprite;

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
extern int lastSeed;

extern int push;
#define pushMax 4

extern bool canDraw;
extern int gameSeconds;

extern bool playAgain;

// extern bool resumeJourney;
// extern bool journeyLevelUp;
// extern bool journeySaveExists;
// extern BlockEngine::Game* journeySave;

extern bool multiplayerStart;

extern int initialLevel;
extern int frameCounter;

extern OBJ_ATTR* titleSprites[2];
extern OBJ_ATTR* queueFrameSprites[3];

#ifndef MULTIBOOT
extern BlockEngine::Bot* testBot;
#endif
extern u8 enemyBoard[4][20][10];

extern int mode;

extern int currentlyPlayingSong;
extern int currentMenu;

extern int previousOptionScreen;
extern bool goToOptions;

extern int rumbleMax;

extern bool rumbleInitialized;
extern bool bigMode;

extern TILE* customSkin;

extern bool proMode;

extern bool gradientEnabled;

#ifndef MULTIBOOT
extern int botThinkingSpeed;
extern int botSleepDuration;
extern int botStepMax;
#endif

extern int clearTimer;
extern int maxClearTimer;

extern BlockEngine::Options* previousGameOptions;

extern FIXED shakeBuff;
extern FIXED shakeVelocity;

extern std::list<BlockEngine::Timestamp*>::iterator replayIterator;
extern std::list<BlockEngine::Timestamp*> currentReplay;

extern BlockEngine::Game* quickSave;
extern bool saveExists;

#define shakeMax 10

extern bool replaying;

extern bool enableBot;
extern bool demo;
extern bool handlingTest;

extern int current[22][12];
extern int previous[22][12];

extern WordSprite* wordSprites[MAX_WORD_SPRITES];

extern std::unique_ptr<Settings> previousSettings;
extern Skin previousSkins[MAX_CUSTOM_SKINS];
extern COLOR* previousPalette;

extern std::list<std::string> path;
extern std::string previousElement;

extern int gridUpdateTimer;
extern int gridUpdateTimerMax;
extern int damp;
extern int dampTimer;
extern int dampTimerMax;

extern int peek;
extern FIXED peekValue;
extern int peekShift;

extern MenuKeys menuKeys;

extern int framesSinceLastSave;

extern bool ended;

extern ReplayHeader* currentReplayHeader;

class Function {
private:
public:
    void (BlockEngine::Game::*gameFunction)(int);
};

struct Packet {
#ifdef GBA
    u8 playerId;
    u16 packet;
    Packet(u8 pId, u16 msg) {
        playerId = pId;
        packet = msg;
    }
#else
    u8 playerId;
    u16 packet;
    Packet(u8 pId, u16 msg) {
        playerId = pId;
        packet = msg;
    }
#endif
};

#ifndef GBA
extern std::string roomId;
#endif
#ifdef PC
std::string getSavefilePath();
#endif
extern std::string exceptionReason;

#ifndef MULTIBOOT
struct StatusData;
extern StatusData status;

#endif
#ifdef GBA
extern interrupt_vector linkVBlankHandler;
#endif

typedef struct HSV_COLOR {
    int hue = 0;
    float saturation = 0;
    float value = 0;
} HSV_COLOR;

COLOR hsv2rgb(int h, float s, float v);
COLOR hsv2rgb(HSV_COLOR hsv);

HSV_COLOR rgb2hsv(int r, int g, int b);

extern std::string updateAvailable;
