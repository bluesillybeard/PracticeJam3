#include "SDL3/SDL_error.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_surface.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#define CELESTIAL_RENDER_PRIV
#include "render.h"

#include "main.h"

// Some extra functions we need if we're on emscripten for web build
#if defined(__EMSCRIPTEN__)
// IMPORTANT NOTE: These assume that the entire window is the game
// At some point, it might be a good idea to change this to get the size of a specific element ID from the html
// But for now, the assumption is that the entire document is just the canvas
// ALSO: See comments in index.html
#include <emscripten.h>
EM_JS(int, document_get_width, (), {
  return window.innerWidth;
});

EM_JS(int, document_get_height, (), {
  return window.innerHeight;
});

#endif

bool celestial_render_init(CelestialState* state) {
    state->render = arena_alloc(&state->permArena, sizeof(CelestialRenderState));

    CelestialRenderState* this = state->render;

    int width = 640;
    int height = 480;
    int windowFlags = SDL_WINDOW_RESIZABLE;

    #if defined(__EMSCRIPTEN__)
    // On emscripten, we actually want to use "fullscreen"
    // By setting the window size to the size of the web page
    // Which will them set the canvas size in the HTML
    // No, setting window maximized does not work for that
    // ALSO: See comments in index.html
    width = document_get_width();
    height = document_get_height();
    #endif

    if (!SDL_CreateWindowAndRenderer("Celestial", width, height, windowFlags, &this->window, &this->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return false;
    }

    SDL_Surface* emscript = SDL_LoadBMP("asset/emscript.bmp");

    if(!emscript) {
        SDL_Log("Could not load emscript: %s", SDL_GetError());
        SDL_Log("cwd: %s", SDL_GetCurrentDirectory());
        return false;
    }

    this->texture = SDL_CreateTextureFromSurface(this->renderer, emscript);
    return true;
}

bool celestial_render_frame(CelestialState* state) {
    CelestialRenderState* this = state->render;

    #if defined(__EMSCRIPTEN__)
    // Emscripten does not resize the canvas if the screen size changes
    // Rather annoying at first, but easy to fix in the end
    // ALSO: See comments in index.html
    int width = document_get_width();
    int height = document_get_height();
    
    int realWidth;
    int realHeight;
    SDL_GetWindowSizeInPixels(this->window, &realWidth, &realHeight);
    // This is to (hopefully) avoid spamming useless resize calls
    // Also, SDL does in fact trigger the resize even upon calling this function
    // TODO: I assume this is always in pixels?
    if(width != realWidth || height != realHeight) {
        SDL_SetWindowSize(this->window, width, height);
    }
    #endif
    
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

    SDL_FRect textureSrc = {.x = 0, .y = 0, .w = 64, .h = 16};
    SDL_FRect textureDst = {.x = 0, .y = 0, .w = 256, .h = 64};
    SDL_RenderTexture(this->renderer, this->texture, &textureSrc, &textureDst);

    SDL_RenderPresent(this->renderer);

    return true;
}
