#pragma once

#include "main.h"
#include <stdbool.h>

#ifdef GAME_PRIV
struct _PracticeJam3GameState {
    float boxPos;
    float lastBoxPos;
};
#endif

bool practiceJam3_game_init(PracticeJam3State* state);

bool practiceJam3_game_frame(PracticeJam3State* state);

bool practiceJam3_game_step(PracticeJam3State* state);
