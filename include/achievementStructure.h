#import "blockEngine.hpp"
#pragma once

struct StatusData {
    [[maybe_unused]] const unsigned int primaryMagic = 1689123404;
    [[maybe_unused]] const unsigned int secondaryMagic = 2035498713;
    // Handling Settings
    [[maybe_unused]] BlockEngine::Tuning tuning;
    // In-game Settings
    [[maybe_unused]] bool *BigMode = nullptr, *ProMode = nullptr, *Paused = nullptr;
    [[maybe_unused]] int *GoalLine = nullptr, *RotationSystem = nullptr, *PeekAbove = nullptr, *UnpauseCountdown = nullptr;
    [[maybe_unused]] int *Randomizer = nullptr, *PlayerLost = nullptr, *PlayerWon = nullptr, *Combo = nullptr;
    [[maybe_unused]] int *ZoneScore = nullptr, *GameMode = nullptr, *SubMode = nullptr;
    [[maybe_unused]] unsigned int *Score = nullptr;
    // Stats
    [[maybe_unused]] BlockEngine::Stats *Stats = nullptr;
    [[maybe_unused]] int *ElapsedTime = nullptr;
    [[maybe_unused]] int *LinesSent = nullptr;
    [[maybe_unused]] BlockEngine::Options *Options;
    [[maybe_unused]] int gradeIndex;
    [[maybe_unused]] std::string gradeString;
    [[maybe_unused]] int *ZoneCharge = nullptr;
    [[maybe_unused]] int *ZoneLines = nullptr;
    [[maybe_unused]] int *ZoneTimer = nullptr;

    // Methods for updating
    void update(Settings* settings) {
        tuning = getTuning();
        UnpauseCountdown = &settings->pauseCountdown;
        Randomizer = &settings->randomizer;
    }
    void startGame() {
        if (game) {
            GoalLine = &game->goal;
            RotationSystem = &game->rotationSystem;
            PeekAbove = &game->peek;
            PlayerLost = &game->lost;
            PlayerWon = &game->won;
            Combo = &game->comboCounter;
            Score = &game->score;
            ZoneScore = &game->zoneScore;
            GameMode = &game->gameMode;
            SubMode = &game->subMode;
            Stats = &game->statTracker;
            ElapsedTime = &game->inGameTimer;
            LinesSent = &game->linesSent;
            ZoneCharge = &game->zoneCharge;
            ZoneLines = &game->zonedLines;
            ZoneTimer = &game->zoneTimer;
        }
        Paused = &paused;
        BigMode = &bigMode;
        ProMode = &proMode;
    }

}
__attribute((aligned(4)));
