#pragma once

#include "SDL3/SDL_events.h"
#include "main.h"
#include <stdbool.h>

bool practiceJam3_game_init(PracticeJam3State* state);

bool practiceJam3_game_frame(PracticeJam3State* state);

bool practiceJam3_game_step(PracticeJam3State* state);

bool practiceJam3_game_event(PracticeJam3State* state, SDL_Event const* ev);
