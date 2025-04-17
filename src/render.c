#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <X11/Xproto.h>

#include "SDL3/SDL_blendmode.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_stdinc.h"
#include "arena.h"
#include "stb_image.h"

#define RENDER_PRIV
#include "render.h"

#define GAME_PRIV
#include "game.h"

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

// other extra functions
static inline float rqlerp(float a, float b, float x) {
    return a * x + b * (1-x);
}

bool practiceJam3_render_init(PracticeJam3State* state) {
    state->render = arena_alloc(&state->permArena, sizeof(PracticeJam3RenderState));
    PracticeJam3RenderState* this = state->render;
    *this = (PracticeJam3RenderState){ 0 };

    int width = 640;
    int height = 480;
    SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;

    #if defined(__EMSCRIPTEN__)
    // On emscripten, we actually want to use "fullscreen"
    // By setting the window size to the size of the web page
    // Which will them set the canvas size in the HTML
    // No, setting window maximized does not work for that
    // ALSO: See comments in index.html
    width = document_get_width();
    height = document_get_height();
    #endif

    if (!SDL_CreateWindowAndRenderer("PracticeJam3", width, height, windowFlags, &this->window, &this->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return false;
    }

    int imageWidth;
    int imageHeight;
    int fileChannels;

    char* filePostfix = "asset/character0.png";
    size_t filePathSizeRequired = strlen(practiceJam3_staticState.virtualCwd) + 1 + strlen(filePostfix) + 1;
    // allocate temporarily
    char* file = arena_alloc(&practiceJam3_staticState.tickArena, filePathSizeRequired);
    SDL_snprintf(file, filePathSizeRequired, "%s/%s", practiceJam3_staticState.virtualCwd, filePostfix);
    stbi_uc* imageResult = stbi_load(file, &imageWidth, &imageHeight, &fileChannels, 4);

    if(!imageResult) {
        SDL_Log("Could not load character0.png: %s", stbi_failure_reason());
        SDL_Log("cwd: %s", SDL_GetCurrentDirectory());
        return false;
    }

    // TODO: probably free the surface and the pixel data
    SDL_Surface* imageSurface = SDL_CreateSurfaceFrom(imageWidth, imageHeight, SDL_PIXELFORMAT_RGBA8888, imageResult, imageWidth*4);

    this->texture = SDL_CreateTextureFromSurface(this->renderer, imageSurface);
    SDL_SetTextureScaleMode(this->texture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(this->texture,SDL_BLENDMODE_BLEND);
    return true;
}

bool practiceJam3_render_frame(PracticeJam3State* state) {
    PracticeJam3RenderState* this = state->render;

    // Value used to interpolate the current state and the last state
    // So if the framerate is higher than the step rate, it still looks smooth
    // 0 -> last state, 1 -> current state
    float interpolator = (float)(state->times.timeNsFrame - state->times.timeNsGame) / (float)nsPerStep;

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

    SDL_SetRenderDrawColor(this->renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(this->renderer);

    SDL_FRect textureSrc = {.x = 0, .y = 0, .w = 128, .h = 128};
    SDL_FRect textureDst = {.x = 0, .y = 0, .w = 256, .h = 256};
    SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderTexture(this->renderer, this->texture, &textureSrc, &textureDst);

    float sillyBoxRadius = 20;
    float sillyBoxPos = rqlerp(state->gameState->boxPos, state->gameState->lastBoxPos, interpolator);
    SDL_FRect sillyBox = {.x = sillyBoxPos - sillyBoxRadius + 80, .y = 60, .w = sillyBoxRadius*2, .h = sillyBoxRadius*2};
    SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(this->renderer, &sillyBox);

    SDL_RenderPresent(this->renderer);

    return true;
}


bool practiceJam3_render_step(PracticeJam3State* state) {
    (void)state;
    return true;
}
