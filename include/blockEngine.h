#pragma once

#include "platform.hpp"
#include <list>
#include <string>
#include "tetromino.hpp"
#include <tuple>

const bool enableFumen = false;

const u8 fumenQueue[5] = {5,0,0,0,0};

const u8 fumenBoard[20][10] = {
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},

    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},

    {1,1,0,0,0, 0,0,0,0,0},
    {1,1,0,0,1, 0,0,0,0,0},
    {1,1,1,1,1, 1,1,0,0,0},
    {1,1,1,1,1, 1,1,1,0,0},
    {1,1,1,1,1, 1,1,1,1,0},

    {1,1,1,1,1, 1,1,1,1,0},
    {1,1,1,1,1, 1,1,1,1,0},
    {1,1,1,0,1, 1,1,1,1,1},
    {1,1,1,1,1, 1,1,1,1,0},
    {1,1,1,1,1, 1,1,1,1,0},
};


namespace BlockEngine
{
    #define MAX_DISAPPEAR 300
    #define CREDITS_LENGTH 3240 //54 seconds * 60
    #define maxSleep 1;

    #define MAX_MOVELIST_SIZE 5

    #define SEARCH_DEPTH 1

    extern const uint16_t connectedConversion[24];
    extern const uint16_t connectedFix[3][24];
    extern int* getShape(int piece,int rotation,int rotationSystem);

    enum Modes{
        NO_MODE,
        MARATHON,
        SPRINT,
        DIG,
        BATTLE,
        ULTRA,
        BLITZ,
        COMBO,
        SURVIVAL,
        CLASSIC,
        MASTER,
        TRAINING,
    };

    enum RotationSystems{
        SRS,
        NRS,
        ARS,
        A_SRS,
    };

    enum Pieces{
        PIECE_I,
        PIECE_J,
        PIECE_L,
        PIECE_O,
        PIECE_S,
        PIECE_T,
        PIECE_Z,
    };

    class Stats{
    public:
        int clears[4] = {0,0,0,0};
        int tspins = 0;
        int perfectClears = 0;
        int maxStreak = 0;
        int maxCombo = 0;
        int holds = 0;
        int maxZonedLines = 0;

        Stats(){}

        Stats(const Stats& old){
            for(int i = 0; i < 4; i++)
                clears[i] = old.clears[i];

            tspins = old.tspins;
            perfectClears = old.perfectClears;
            maxStreak = old.maxStreak;
            maxCombo = old.maxCombo;
            holds = old.holds;
            maxZonedLines = old.maxZonedLines;
        }
    };

    class Color {
    public:
        int r;
        int g;
        int b;
    };
    
    class Drop {
    public:
        int on = false;
        int startX = 0;
        int endX = 0;
        int startY = 0;
        int endY = 0;
        int rawEndY = 0;

        int x;
        int y;

        int dx;
        int dy;

        int piece;
        int rotation;
        int rotating;


        Drop(){}
        Drop(const Drop& oldDrop){
            on = oldDrop.on;
            startX = oldDrop.startX;
            endX = oldDrop.endX;
            startY = oldDrop.startY;
            endY = oldDrop.endY;
            rawEndY = oldDrop.rawEndY;
            x = oldDrop.x;
            y = oldDrop.y;
            dx = oldDrop.dx;
            dy = oldDrop.dy;
            piece = oldDrop.piece;
            rotation = oldDrop.rotation;
            rotating = oldDrop.rotating;
        }
    };

    class Move{
    public:
        int dx = 0;
        int rotation = 0;
        int height = 40;
        int holes = 0;
        int piece = -1;
        int clears = 0;
        // int wells = 0;
        int score = -0x7fffffff;

        Move(){}

        Move(const Move& oldMove){
            dx = oldMove.dx;
            rotation = oldMove.rotation;
            height = oldMove.height;
            holes = oldMove.holes;
            piece = oldMove.piece;
            clears = oldMove.clears;
            score = oldMove.score;
        }
    };

    class Garbage {
    public:
        int id;
        int amount;
        int timer = 60;

        Garbage(int _id,int _amount){
            id = _id;
            amount = _amount;
        }
    };

    class Score {
    public:
        int linesCleared = 0;
        int score = 0;
        int combo = 0;
        int isTSpin = 0;
        int isPerfectClear = 0;
        int isBackToBack = 0;
        int isDifficult = 0;
        Drop drop;

        Score(){}
        Score(int lC, int sc, int co, int isTS, int isPC, int isBTB, int isDif, Drop drp) {
            linesCleared = lC;
            score = sc;
            combo = co;
            isTSpin = isTS;
            isPerfectClear = isPC;
            isBackToBack = isBTB;
            isDifficult = isDif;
            drop = drp;
        }

        Score(const Score& oldScore){
            linesCleared = oldScore.linesCleared;
            score = oldScore.score;
            combo = oldScore.combo;
            isTSpin = oldScore.isTSpin;
            isPerfectClear = oldScore.isPerfectClear; 
            isBackToBack = oldScore.isBackToBack;
            isDifficult = oldScore.isDifficult;
            drop = Drop(oldScore.drop);
        }
    };

    class SoundFlags {
    public:
        int shift = 0;
        int rotate = 0;
        int place = 0;
        int invalid = 0;
        int hold = 0;
        int clear = 0;
        int levelUp = 0;
        int finesse = 0;
        int section = 0;
        int disappear = 0;
        int zone = 0;
        int meter = 0;
        int initial = 0;
        int tspin = 0;

        SoundFlags() {}
        SoundFlags(const SoundFlags& oldFlags){
            shift = oldFlags.shift;
            rotate = oldFlags.rotate;
            place = oldFlags.place;
            invalid = oldFlags.invalid;
            hold = oldFlags.hold;
            clear = oldFlags.clear;
            levelUp = oldFlags.levelUp;
            finesse = oldFlags.finesse;
            section = oldFlags.section;
            disappear = oldFlags.disappear;
            zone = oldFlags.zone;
            meter = oldFlags.meter;
            initial = oldFlags.initial;
            tspin = oldFlags.tspin;
        }
    };

    class Pawn{
    public:
        int x;
        int y;
        int type;
        int current = -1;
        int rotation = 0;
        int board[4][4][4];
        int boardLowest[4][4];
        int heighest[4];
        int lowestBlock[4];
        int lowest;
        bool big = false;
        void setBlock(int system);

        Pawn(int newX, int newY) {
            for(int i = 0; i < 4; i++){
                heighest[i] = -1;
                for(int j = 0; j < 4; j++)
                    boardLowest[i][j] = -1;
            }

            x = newX;
            y = newY;
        }

        Pawn(const Pawn& oldPawn){
            x = oldPawn.x;
            y = oldPawn.y;

            type = oldPawn.type;
            current = oldPawn.current;
            rotation = oldPawn.rotation;
            lowest = oldPawn.lowest;
            big = oldPawn.big;

            memcpy32(board, oldPawn.board, 4 * 4 * 4);
            memcpy32(boardLowest, oldPawn.boardLowest, 4 * 4);
            memcpy32(lowestBlock, oldPawn.lowestBlock, 4);
            memcpy32(heighest, oldPawn.heighest, 4);
        }
    };

    class Tuning{
        public:
            int das = 8;
            int arr = 2;
            int sfr = 2;
            int dropProtection = 8;
            bool directionalDas = false;
            bool delaySoftDrop = true;
            bool ihs = false;
            bool irs = false;
            bool initialType = false;
    };

    class Options{
        public:
            Modes mode = NO_MODE;
            int goal = 0;
            int level = 0;
            Tuning tuning;

            bool trainingMode = false;
            bool bigMode = false;

            int bTypeHeight = 0;
            int subMode = 0;
            int rotationSystem = SRS;
    };

    class Timestamp{
    public:
        int timer;
        uint16_t move;
        bool dir;
        bool shiftTimer = false;

        Timestamp(int _timer, int _move, int _dir){
            timer = _timer;
            move = _move;
            dir = _dir;
        }

        Timestamp(bool shift){
            shiftTimer = shift;
        }

        Timestamp(uint32_t m){
            timer = (m >> 11) & 0x3fffff;
            dir = (m >> 9) & 1;
            move = m & 0x1ff;
        }
    };

    class Game {
    private:
        void fillBag();
        void fillQueue(int);
        void fillQueueSeed(int,int);
        bool moveLeft();
        bool moveRight();
        void moveDown();
        int clear(Drop);
        void lockCheck();
        void next();
        void place();
        void generateGarbage(int,int);
        Drop calculateDrop();
        void setMasterTuning();
        void rotate(int,int);
        int checkSpecialRotation(int,int);
        int checkITouching(int,int);
        void rotatePlace(int,int,int,int);
        void updateDisappear();
        void endZone();
        void clearBoard();
        void fixConnected(std::list<int>);
        void connectBoard(int startY, int endY);
        void updateColumns(int height);

        int getNbr(int x, int y, int ex, int sy, int ey);

        int bigBag[35];

        float speedCounter = 0;

        //7  117
        //8  133
        //9  150 
        //11 183
        //16 267

        int maxDas = 8;
        int das = 0;
        int arr = 2;
        int arrCounter = 0;

        int softDropCounter = 0;
        int maxSoftDrop = 10;
        int softDropSpeed = 2;
        int softDropRepeatTimer = 0;

        int lockMoveCounter = 15;

        int left = 0;
        int right = 0;
        int down = 0;

        int lastMoveRotation = 0;
        int lastMoveDx = 0;
        int lastMoveDy = 0;

        int finesseCounter = 0;

        Drop lastDrop;

        bool dropProtection = true;
        bool directionCancel = true;

        int dropLockTimer = 0;
        int dropLockMax = 8;

        bool specialTspin = false;

        int pieceHistory = -1;

        int gracePeriod = 0;

        int rotating = 0;

        int section = 0;
        int sectionStart = 0;
        int previousSectionTime = 1000000;

        int internalGrade = 0;
        int gradePoints = 0;

        bool cool = false;
        bool regret = false;
        int decayTimer = 0;

        bool stopLockReset = false;
        bool fromLockHold = false;
        bool fromLockRotate = false;

        int pieceDrought[7];
        bool delaySoftDrop = true;

        bool ihs = false;
        bool irs = false;
        bool initialType = false;

        bool rotates[3] = {false,false,false};

        bool zoneExit = false;

        std::list<int> bag;
        std::list<int> historyList;

    public:
        int lengthX = 10;
        int lengthY = 40;
        int board[40][10];

        uint16_t bitboard[40];
        int columnHeights[10];

        Pawn pawn = Pawn(0, 0);
        int held = -1;
        float speed;
        int linesCleared = 0;
        int level = 0;
        int score = 0;
        int comboCounter = 0;
        bool lineClearArray[40];
        int clearLock = 0;
        int lost = 0;
        int gameMode;
        SoundFlags sounds;
        Score previousClear = Score(0, 0, 0, 0, 0, 0, 0, Drop());
        int timer = 0;
        int refresh = 0;
        int won = 0;
        int goal = 40;
        int finesseFaults = 0;
        int garbageCleared = 0;
        int garbageHeight = 0;
        int pushDir = 0;
        int b2bCounter = 0;
        int bagCounter = 0;
        int linesSent = 0;
        int moveCounter = 0;
        int pieceCounter = 0;
        int previousKey = 0;
        bool softDrop = false;
        bool canHold = true;
        bool holding = false;
        int holdCounter = 0;
        bool trainingMode = false;
        int seed = 0;
        int initSeed = 0;
        int initialLevel = 0;
        int eventTimer = 0;

        bool dropping = false;
        int entryDelay = 0;
        int areMax = 0;
        int lineAre = 0;
        int maxClearDelay = 1;

        int bTypeHeight = 0;

        Stats statTracker;

        int subMode = 0;
        int zoneCharge = 0;
        int zoneTimer = 0;
        int zonedLines = 0;
        int zoneClearCounter = 0;
        int zoneScore = 0;
        int zoneStart = 0;
        bool fullZone = false;
        bool inversion = false;

        int grade = 0;
        int coolCount = 0;
        int regretCount = 0;

        int rotationSystem = SRS;

        int maxLockTimer = 30;
        int lockTimer = maxLockTimer;

        uint16_t disappearTimers[40][10];
        int disappearing = 0;

        float creditGrade = 0;

        bool eventLock = false;

        int finesseStreak = 0;

        int inGameTimer = 0;

        int stackHeight = lengthY;

        int replayTimerSections = 0;

        int boardOffset = lengthY/2;

        int activePiece = -1;

        bool toEndZone = false;

        int verticalKick = 0;
        int verticalKickMax = 8;

        bool replayElligible = true;

        int fullLine = 0x3ff;

        std::list<int> queue;
        std::list<int> linesToClear;
        std::list<int> moveHistory;
        std::list<int> previousBest;
        std::list<Garbage> attackQueue;
        std::list<Garbage> garbageQueue;
        std::list<std::tuple<uint8_t,uint8_t>> toDisappear;
        std::list<Timestamp> replay;

        int checkRotation(int, int, int);
        void rotateCW(int dir);
        void rotateCCW(int dir);
        void rotateTwice(int dir);
        void hardDrop();
        void update();
        int lowest();
        Color color(int);
        void hold(int dir);
        void keyLeft(int);
        void keyRight(int);
        void keyDown(int);
        void keyDrop(int);
        void removeClearLock();
        void resetSounds();
        void resetRefresh();
        void setLevel(int);
        void setGoal(int);
        Drop getDrop();
        void setTuning(Tuning);
        void clearAttack(int);
        void setWin();
        void addToGarbageQueue(int,int);
        std::list<int> getBestFinesse(int,int,int);
        int getIncomingGarbage();
        Move findBestDrop();
        void setTrainingMode(bool);
        void demoClear();
        void demoFill();
        void bType(int);
        void setSubMode(int);
        void setSpeed();
        void setRotationSystem(int);
        void removeEventLock();
        void activateZone(int);
        void liftKeys();
        void setOptions(Options newOptions);
        void updateDas();
        void chargeDas();

        void loadFumen();

        Game(){
            seed = initSeed = qran();
        }

        Game(int gm, int sd, bool big){
            gameMode = gm;
            seed = initSeed = sd;

            for (int i = 0; i < 40; i++) {
                bitboard[i] = 0;
                lineClearArray[i] = false;

                for (int j = 0; j < 10; j++)
                    board[i][j] = 0;
            }

            for(int j = 0; j < lengthX; j++)
                columnHeights[j] = 0;

            fillBag();

            if(gameMode == CLASSIC){
                maxLockTimer = 1;
                fillQueue(1);

                maxDas = 16;
                arr = 6;
                softDropSpeed = 2;
                gracePeriod = 90;

                rotationSystem = NRS;
            }else if(enableFumen){
                for(int i = 0; i < 5; i++)
                    queue.push_back(fumenQueue[i]);
            } else
                fillQueue(5);

            pawn = Pawn(0,0);
            pawn.big = big;

            if(gameMode == SPRINT)
                goal = 40;
            else if(gameMode == MARATHON)
                goal = 150;
            else if(gameMode == DIG){
                goal = 100;
                if(!big)
                    generateGarbage(9,0);
                else
                    generateGarbage(4,0);
            }else if(gameMode == COMBO){
                if(!big){
                    for(int i = lengthY/2-4; i < lengthY; i++){
                        for(int j = 0; j < 10; j++){
                            if(j > 2 && j < 7 && !(i == lengthY-2 && j < 5) && !(i == lengthY-1 && j < 4))
                                continue;
                            board[i][j] = i % 7 + 1;
                            bitboard[i] |= 1 << j;

                            if((lengthY-i) > columnHeights[j])
                                columnHeights[j] = lengthY-i;
                        }
                        connectBoard(i, i);
                    }
                    stackHeight = lengthY/2 - 4;
                }else{
                    for(int i = (lengthY/4-1); i < lengthY/2; i++){
                        for(int j = 0; j < 5; j++){
                            if(j != 0 && !(i == (lengthY/2)-2 && j < 3) && !(i == (lengthY/2)-1 && j < 2))
                                continue;
                            board[i*2][j*2] = i % 7 + 1;
                            board[i*2][j*2+1] = i % 7 + 1;
                            board[i*2+1][j*2] = i % 7 + 1;
                            board[i*2+1][j*2+1] = i % 7 + 1;
                        }
                    }
                }
            }else if(gameMode == MASTER){
                level = 0;
                speed = GameInfo::masterGravity[0][1];
                setMasterTuning();
            }

            if(big){
                lengthY /= 2;
                lengthX /= 2;
                boardOffset = lengthY/2;
                fullLine >>= 5;
            }

            for (int i = 0; i < 40; i++) {
                for (int j = 0; j < 10; j++)
                    disappearTimers[i][j] = 0;
            }

            if(enableFumen)
                loadFumen();
        }

        Game(int gm,bool big) : Game(gm,qran(),big){}

        Game(const Game& oldGame){
            for (int i = 0; i < 40; i++) {
                bitboard[i] = oldGame.bitboard[i];
                for (int j = 0; j < 10; j++)
                    board[i][j] = oldGame.board[i][j];

                lineClearArray[i] = oldGame.lineClearArray[i];
            }

            for (int j = 0; j < 10; j++)
                columnHeights[j] = oldGame.columnHeights[j];

            queue = oldGame.queue;
            canHold = oldGame.canHold;
            bag = oldGame.bag;

            pawn = Pawn(oldGame.pawn);
            statTracker = Stats(oldGame.statTracker);

            held = oldGame.held;
            level = oldGame.level;
            score = oldGame.score;
            comboCounter = oldGame.comboCounter;
            linesCleared = oldGame.linesCleared;
            clearLock = oldGame.clearLock;
            gameMode = oldGame.gameMode;
            // sounds = SoundFlags(oldGame.sounds);
            previousClear = Score(oldGame.previousClear);
            timer = oldGame.timer;
            inGameTimer = oldGame.inGameTimer;
            refresh = oldGame.refresh;
            won = oldGame.won;
            lost = oldGame.lost;
            goal = oldGame.goal;
            finesseFaults = oldGame.finesseFaults;
            garbageCleared = oldGame.garbageCleared;
            garbageHeight = oldGame.garbageHeight;
            pushDir = oldGame.pushDir;
            b2bCounter = oldGame.b2bCounter;
            bagCounter = oldGame.bagCounter;
            seed = oldGame.seed;
            initSeed = oldGame.initSeed;
            subMode = oldGame.subMode;
            initialLevel = oldGame.initialLevel;
            trainingMode = oldGame.trainingMode;
            clearLock = oldGame.clearLock;
            linesToClear = oldGame.linesToClear;
            linesCleared = oldGame.linesCleared;
            dropping = oldGame.dropping;
            stackHeight = oldGame.stackHeight;
            pieceCounter = oldGame.pieceCounter;
            speed = oldGame.speed;
            activePiece = oldGame.activePiece;
        }


        ~Game(){}
    };

    typedef struct Weights{
        int clears = 0;
        int holes = 0;
        int height = 0;
        int lowest = 0;
        int jag = 0;
        int well = 0;
        int row = 0;
        int col = 0;
        int quadWell = 0;
    }Weights;

    typedef struct Values{
        int rowTrans;
        int colTrans;
        int holes;
        int wells;
        int quadWell;
        int wellX;
    }Values;

    class Bot{
    public:
        bool thinking = true;
        int thinkingI = -6;
        int thinkingJ = 0;
        Move current;
        Move best;
        int sleepTimer = 0;
        int exploring = 0;

        int dx = 0;
        int rotation = 0;
        Game* game;
        Game* testGame = nullptr;
        // uint16_t testBoard[40];
        // int columnHeights[10];

        Values currentValues;

        std::list<Move> moves;
        Move firstMoves[MAX_MOVELIST_SIZE];
        Move bestMoves[MAX_MOVELIST_SIZE];
        int bestFuture = 0;

        int checking = 0;
        int previous = 0;

        Weights weights{
        1,//clears
        -4,//holes
        0,
        -1,//lowest
        0,
        -1,//well
        -1,//row
        -1,//col
        8,//quadwell
        };

        void run();

        void move();

        // int findBestDrop(int ii,int jj, Game * game);

        void stopThinking();

        void executeMove(Move move);

        Bot(){}
        Bot(Game* _game){
            game = _game;
            testGame = new Game(*game);
        }

        ~Bot(){
            if(testGame != nullptr)
                delete testGame;
        }
    };

}
