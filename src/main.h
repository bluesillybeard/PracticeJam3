#pragma once
// PracticeJam3's "global" header file
// I suspect nearly every PracticeJam3 source file will include this in one way or another

#include "ext/arena.h"
#include <stdbool.h>

// Global compile-time settings
static const int64_t nsPerStep = (1000 * 1000 * 1000) / 100; // 100 hz as a delay in nano seconds

// Opaque structs for subsystems
typedef struct _PracticeJam3RenderState PracticeJam3RenderState;
typedef struct _PracticeJam3GameState PracticeJam3GameState;

// transparent types for clarity and structuring
typedef struct _PracticeJam3TimingData {
    /// @brief the timestamp in nano seconds when the current frame was called.
    /// Do not use this for game logic, as it will not be consistent in any manner relative to game time.
    int64_t timeNsFrame;
    /// @brief the timestamp in nano seconds when the last frame was called.
    /// Do not use this for game logic, as it will not be consistent in any manner relative to game time.
    int64_t timeNsLastFrame;
    /// @brief the real (as in real-world) timestamp in nano seconds when the current step was called.
    /// Do not use for game logic - it will appear to work, right up until lag occurs when the game time and real time drift apart.
    int64_t timeNsStep;
    /// @brief the real (as in real-world) timestamp in nano seconds when the previous step was called.
    /// Do not use for game logic - it will appear to work, right up until lag occurs when the game time and real time drift apart.
    int64_t timeNsLastStep;
    /// @brief the current timestamp in nano seconds as far as the game is concerned.
    /// Just an arbitrary monotonic timer like everything else in this struct - but it counts up per-step instead of in realtime.
    int64_t timeNsGame;
} PracticeJam3TimingData;

typedef struct {
    // Global components
    PracticeJam3TimingData times;
    /// @brief A permanent memory arena - ANYTHING ALLOCATED HERE IS NOT FREED UNTIL THE GAME FULLY EXITS.
    Arena permArena;
    /// @brief An arena that is cleared at the end of each rendered frame
    Arena frameArena;
    /// @brief An arena that is cleared at the end of each game tick
    Arena tickArena;
    bool closing;
    // Subsystems
    PracticeJam3RenderState* render;
    PracticeJam3GameState* gameState;
    // Some stupid things that have to exist for stupid reasons
    char* virtualCwd;
} PracticeJam3State;

extern PracticeJam3State practiceJam3_staticState;
