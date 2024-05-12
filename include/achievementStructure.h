#import "blockEngine.hpp"
#pragma once

struct StatusData {
    [[maybe_unused]] const unsigned int primaryMagic = 1689123404;
    [[maybe_unused]] const unsigned int secondaryMagic = 2035498713;
    // Handling Settings
    [[maybe_unused]] BlockEngine::Tuning tuning;
    // In-game Settings
    [[maybe_unused]] bool *BigMode = nullptr, *ProMode = nullptr;
    [[maybe_unused]] int *GoalLine = nullptr, *RotationSystem = nullptr, *PeekAbove = nullptr, *UnpauseCountdown = nullptr;
    [[maybe_unused]] int *Randomizer = nullptr, *PlayerLost = nullptr, *PlayerWon = nullptr;

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
        }
        BigMode = &bigMode;
        ProMode = &proMode;
    }
}
__attribute((aligned(4)));
