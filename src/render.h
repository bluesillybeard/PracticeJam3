#pragma once
#include <stdbool.h>
#include "SDL3/SDL_render.h"
#include "main.h"

#ifdef RENDER_PRIV

typedef struct _RenderCommand {
    float x;
    float y;
    float w;
    float h;
    SDL_Texture* texture;
    float tintRed;
    float tintGreen;
    float tintBlue;
    float tintAlpha;
} RenderCommand;

typedef struct _RenderLayer {
    RenderCommand* cmds;
    size_t count;
    size_t capacity;
    int depth;
} RenderLayer;

struct _PracticeJam3RenderState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool closing;
    // camera center in world space
    float cameraCenterX;
    float cameraCenterY;
    // Camera radius in world space
    float cameraRadius;
    float cameraCenterXLast;
    float cameraCenterYLast;
    float cameraRadiusLast;
    // Camera settings from the last setCamera()
    float cameraRadiusSet;
    float cameraCenterXSet;
    float cameraCenterYSet;
    // list of render layers
    RenderLayer* layers;
    size_t numRenderLayers;
};
#endif

bool practiceJam3_render_init(PracticeJam3State* state);

bool practiceJam3_render_frame(PracticeJam3State* state);

void practiceJam3_render_setCamera(PracticeJam3State* state, float centerX, float centerY, float radius);

bool practiceJam3_render_step(PracticeJam3State* state);

SDL_Texture* practiceJam3_render_loadTexture(PracticeJam3State* state, char* assetPath);

bool practiceJam3_render_sprite(PracticeJam3State* state, float x, float y, float w, float h, SDL_Texture* texture, float tintRed, float tintGreen, float tintBlue, float tintAlpha, int layer);

// Value used to interpolate the current state and the last state
// So if the framerate is higher than the step rate, it still looks smooth
// 0 -> last state, 1 -> current state
// This effectively a measurement of how far into the step we are
float practiceJam3_render_getInterpolator(PracticeJam3State* state);
