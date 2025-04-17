#pragma once
#include <stdbool.h>
#include "main.h"

#ifdef RENDER_PRIV
struct _PracticeJam3RenderState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool closing;
    SDL_Texture* characterTexture;
    // camera center in world space
    float cameraCenterX;
    float cameraCenterY;
    // Camera radius in world space
    float cameraRadius;
    float cameraCenterXLast;
    float cameraCenterYLast;
    float cameraRadiusLast;
};
#endif

bool practiceJam3_render_init(PracticeJam3State* state);

bool practiceJam3_render_frame(PracticeJam3State* state);

bool practiceJam3_render_step(PracticeJam3State* state);
