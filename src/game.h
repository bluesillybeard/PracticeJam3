#pragma once

#include "SDL3/SDL_events.h"
#include "main.h"
#include <stdbool.h>

#ifdef GAME_PRIV
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_rect.h"

// This struct exists in case I want to add more types of colliders
typedef struct _GameCollider {
    SDL_FRect AABB;
} GameCollider;

typedef struct _GameScene {
    char* backdrop;
    SDL_FRect backdropDst;
    float playerStartX;
    float playerStartY;
    GameCollider const* colliders;
    size_t numColliders;
} GameScene;

struct _PracticeJam3GameState {
    float playerX;
    float playerY;
    float playerXLast;
    float playerYLast;
    bool upControlState;
    bool downControlState;
    bool leftControlState;
    bool rightControlState;
    SDL_Texture* characterTexture;
    SDL_Texture* scene1Texture;
    bool showHitboxes;
};
#endif

bool practiceJam3_game_init(PracticeJam3State* state);

bool practiceJam3_game_frame(PracticeJam3State* state);

bool practiceJam3_game_step(PracticeJam3State* state);

bool practiceJam3_game_event(PracticeJam3State* state, SDL_Event const* ev);
