#include "SDL3/SDL_events.h"
#include "main.h"
#include <unistd.h>
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

bool practiceJam3_game_event(PracticeJam3State* state, SDL_Event const* ev) {
    PracticeJam3GameState* this = state->gameState;
    // Gotta love fall-through by default
    switch (ev->type) {
        case SDL_EVENT_KEY_DOWN: {
            // TODO: probably make this more robust
            switch (ev->key.key) {
                case SDLK_W: this->upControlState = true; break;
                case SDLK_A: this->leftControlState = true; break;
                case SDLK_S: this->downControlState = true; break;
                case SDLK_D: this->rightControlState = true; break;
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            switch (ev->key.key) {
                case SDLK_W: this->upControlState = false; break;
                case SDLK_A: this->leftControlState = false; break;
                case SDLK_S: this->downControlState = false; break;
                case SDLK_D: this->rightControlState = false; break;
            }
            break;
        }
    }
    return true;
}

bool practiceJam3_game_step(PracticeJam3State* state) {
    PracticeJam3GameState* this = state->gameState;
    this->playerXLast = this->playerX;
    this->playerYLast = this->playerY;
    float delta = (float)nsPerStep / (1000.0f*1000.0f*1000.0f);
    float const playerSpeed = 4;
    if(this->upControlState) {
        this->playerY -= playerSpeed * delta;
    }
    if(this->downControlState) {
        this->playerY += playerSpeed * delta;
    }
    if(this->rightControlState) {
        this->playerX += playerSpeed * delta;
    }
    if(this->leftControlState) {
        this->playerX -= playerSpeed * delta;
    }
    return true;
}
