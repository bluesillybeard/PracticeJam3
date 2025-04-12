#pragma once
// Celestial's "global" header file
// I suspect nearly every Celestial source file will include this in one way or another

#include "arena.h"

// Opaque structs for subsystems
typedef struct _CelestialRenderState CelestialRenderState;

typedef struct {
    // Global components
    /// @brief A permanent arena - ANYTHING ALLOCATED HERE IS NOT FREED UNTIL THE GAME FULLY EXITS.
    Arena permArena;
    /// @brief An arena that is cleared at the end of each rendered frame
    Arena frameArena;
    /// @brief An arena that is cleared at the end of each game tick
    Arena tickArena;
    // Subsystems
    CelestialRenderState* render;
} CelestialState;

extern CelestialState celestial_staticState;
