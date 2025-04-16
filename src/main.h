#pragma once
// PracticeJam3's "global" header file
// I suspect nearly every PracticeJam3 source file will include this in one way or another

#include "arena.h"

// Opaque structs for subsystems
typedef struct _PracticeJam3RenderState PracticeJam3RenderState;

typedef struct {
    // Global components
    /// @brief A permanent arena - ANYTHING ALLOCATED HERE IS NOT FREED UNTIL THE GAME FULLY EXITS.
    Arena permArena;
    /// @brief An arena that is cleared at the end of each rendered frame
    Arena frameArena;
    /// @brief An arena that is cleared at the end of each game tick
    Arena tickArena;
    // Subsystems
    PracticeJam3RenderState* render;
} PracticeJam3State;

extern PracticeJam3State practiceJam3_staticState;
