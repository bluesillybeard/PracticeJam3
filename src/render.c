#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#define CELESTIAL_RENDER_PRIV
#include "render.h"

#include "main.h"

bool celestial_render_init(CelestialState* state) {
    state->render = arena_alloc(&state->permArena, sizeof(CelestialRenderState));

    CelestialRenderState* this = state->render;

    if (!SDL_CreateWindowAndRenderer("Celestial", 640, 480, SDL_WINDOW_RESIZABLE, &this->window, &this->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool celestial_render_frame(CelestialState* state) {
    CelestialRenderState* this = state->render;
    
    SDL_FRect rect;

    SDL_SetRenderDrawColor(this->renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(this->renderer);

    SDL_SetRenderDrawColor(this->renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    rect.x = rect.y = 100;
    rect.w = 440;
    rect.h = 280;
    SDL_RenderFillRect(this->renderer, &rect);

    SDL_SetRenderDrawColor(this->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);  /* red, full alpha */

    SDL_SetRenderDrawColor(this->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);  /* green, full alpha */
    rect.x += 30;
    rect.y += 30;
    rect.w -= 60;
    rect.h -= 60;
    SDL_RenderRect(this->renderer, &rect);

    SDL_SetRenderDrawColor(this->renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);  /* yellow, full alpha */
    SDL_RenderLine(this->renderer, 0, 0, 640, 480);
    SDL_RenderLine(this->renderer, 0, 480, 640, 0);

    SDL_RenderPresent(this->renderer);  /* put it all on the screen! */

    return true;
}
