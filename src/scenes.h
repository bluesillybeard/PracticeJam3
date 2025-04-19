#pragma once

// This file exists because there are too many scenes to just shove into the main.c file directly
#include "SDL3/SDL_stdinc.h"
#define GAME_PRIV
#include "game.h"

#define AABB(x0, y0, x1, y1) (GameCollider) {.AABB = { SDL_min(x0, x1), SDL_min(y0, y1), SDL_max(x0, x1)-SDL_min(x0, x1), SDL_max(y0, y1)-SDL_min(y0, y1) }}

static const GameCollider scene1Colliders[] = {
    AABB(-1.4f, -2.3f, 12.0f, -1.4f), // top wall
    AABB(-1.4f, -2.3f, -0.8f, 6.5f), // left wall
    AABB(-1.4f, 7.3f, 12.0f, 6.5f), // bottom wall
    AABB(12.8f, 7.3f, 12.0f, -2.3f), // right wall
    AABB(0.6f, -0.8f, 1.7f, 0.9f), // top-left desk
    AABB(4.8f, -0.8f, 5.9f, 0.9f), // top-right desk
    AABB(0.6f, 3.4f, 1.7f, 5.2f), // bottom-left desk
    AABB(4.8f, 3.4f, 5.9f, 5.2f), // bottom-right desk
    AABB(8.9f, -1.4f, 9.9f, 4.3f), // teacher's desk
};

static const GameScene scene1 = {
    .backdrop = "asset/classroom0.png",
    .backdropDst = { -1.4f, -2.3f,  1.4f+12.8f, 2.3f+7.3f },
    .playerStartX = -0.5f,
    .playerStartY = -0.5f,
    .colliders = scene1Colliders,
    .numColliders = sizeof(scene1Colliders)/sizeof(scene1Colliders[0]),
};
