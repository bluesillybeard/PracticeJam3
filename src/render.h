#pragma once
#include <stdbool.h>
#include "main.h"

#ifdef CELESTIAL_RENDER_PRIV
struct _CelestialRenderState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool closing;
    SDL_Texture* texture;
};
#endif

bool celestial_render_init(CelestialState* state);

bool celestial_render_frame(CelestialState* state);
