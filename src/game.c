#include "SDL3/SDL_stdinc.h"
#include "main.h"
#define GAME_PRIV
#include "game.h"
#include <stdbool.h>

bool practiceJam3_game_init(PracticeJam3State* state) {
    state->gameState = arena_alloc(&state->permArena, sizeof(PracticeJam3GameState));
    *state->gameState = (PracticeJam3GameState){0};
    return true;
}

bool practiceJam3_game_frame(PracticeJam3State* state) {
    (void)state;
    return true;
}

bool practiceJam3_game_step(PracticeJam3State* state) {
    PracticeJam3GameState* this = state->gameState;
    // 10 seconds
    int64_t period = 1000l*1000l*1000l * 10l;
    this->lastBoxPos = this->boxPos;
    this->boxPos = 50 * SDL_sinf(((float)(state->times.timeNsGame % period) / (float)period) * 2 * SDL_PI_F);
    return true;
}
