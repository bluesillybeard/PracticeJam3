#include "SDL3/SDL_events.h"
#include "cglm/util.h"
#include "main.h"
#include "render.h"
#include <unistd.h>
#define GAME_PRIV
#include "game.h"
#include <stdbool.h>

bool practiceJam3_game_init(PracticeJam3State* state) {
    state->gameState = arena_alloc(&state->permArena, sizeof(PracticeJam3GameState));
    PracticeJam3GameState* this = state->gameState;
    *this = (PracticeJam3GameState){0};

    this->characterTexture = practiceJam3_render_loadTexture(state, "asset/character0.png");
    SDL_SetTextureScaleMode(this->characterTexture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(this->characterTexture,SDL_BLENDMODE_BLEND);
    return true;
}

bool practiceJam3_game_frame(PracticeJam3State* state) {
    PracticeJam3GameState* this = state->gameState;
    float interpolator = practiceJam3_render_getInterpolator(state);
    float playerX = glm_lerpc(this->playerXLast, this->playerX, interpolator);
    float playerY = glm_lerpc(this->playerYLast, this->playerY, interpolator);
    if(!practiceJam3_render_sprite(state, playerX, playerY, 1, 1, this->characterTexture, 1, 1, 1, 1, 0)) {
        return false;
    }
    return true;
}

bool practiceJam3_game_event(PracticeJam3State* state, SDL_Event const* ev) {
    PracticeJam3GameState* this = state->gameState;
    // Gotta love fall-through by default
    switch (ev->type) {
        case SDL_EVENT_KEY_DOWN: {
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

    practiceJam3_render_setCamera(state, this->playerX+0.5f, this->playerY+0.5f, 12);
    return true;
}
