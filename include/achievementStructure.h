#import "blockEngine.hpp"
#pragma once

struct ExternalSettings {
    // Handling Settings
    [[maybe_unused]] u8 DAS, ARR, SFR, DirectionalDelay, PreviewCount, DropProtection;
    [[maybe_unused]] u8 DisableDiagonals, DelaySoftDrop, InitialHold, InitialRotation, InitialSystem;
    // In-game Settings
    [[maybe_unused]] u8 BigMode, ProMode, GoalLine, RotationSystem, PeekAbove, UnpauseCountdown;
    [[maybe_unused]] u8 Randomizer, SpawnPreview, PlayerAlive;

    // Methods for updating
    void update(Settings* settings) {
        BlockEngine::Tuning tuning = getTuning();
        DAS = tuning.das;
        ARR = tuning.arr;
        SFR = tuning.sfr;
        DirectionalDelay = tuning.directionalDas;
        DropProtection = tuning.dropProtection;

        PreviewCount = settings->maxQueue;
        DisableDiagonals = settings->noDiagonals;
        DelaySoftDrop = settings->delaySoftDrop;
        InitialHold = settings->ihs;
        InitialRotation = settings->irs;
        InitialSystem = settings->initialType;
        UnpauseCountdown = settings->pauseCountdown;
        Randomizer = settings->randomizer;
        //SpawnPreview = settings->; no idea...
    }
    void update() {
        if (game) {
            GoalLine = game->goal;
            RotationSystem = game->rotationSystem;
            PeekAbove = game->peek;
            PlayerAlive = !(game->won || game->lost);
        }
        BigMode = bigMode;
        ProMode = proMode;
    }
}
__attribute((aligned(4)));

struct StatusData {
    [[maybe_unused]] const unsigned int primaryMagic = 1689123404;
    [[maybe_unused]] const unsigned int secondaryMagic = 2035498713;
    struct ExternalSettings* externalSettings;

    explicit StatusData(Settings* settings) {
        externalSettings = new ExternalSettings();
        externalSettings->update(settings);
    }

    ~StatusData() {
        delete externalSettings;
    }
}
__attribute((aligned(4)));
