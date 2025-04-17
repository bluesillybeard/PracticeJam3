#pragma once

#include "SDL3/SDL_events.h"
#include "main.h"
#include <stdbool.h>

#ifdef GAME_PRIV

struct _PracticeJam3GameState {
    float playerX;
    float playerY;
    float playerXLast;
    float playerYLast;
    bool upControlState;
    bool downControlState;
    bool leftControlState;
    bool rightControlState;
};
#endif

bool practiceJam3_game_init(PracticeJam3State* state);

bool practiceJam3_game_frame(PracticeJam3State* state);

bool practiceJam3_game_step(PracticeJam3State* state);

bool practiceJam3_game_event(PracticeJam3State* state, SDL_Event const* ev);
