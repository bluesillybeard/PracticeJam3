#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>

#include "arena.h"
#include "cglm/call/affine.h"
#include "cglm/call/mat4.h"
#include "cglm/types.h"
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

static SDL_Texture* loadTexture(PracticeJam3RenderState* this, char* assetPath) {
    int imageWidth;
    int imageHeight;
    int fileChannels;

    size_t filePathSizeRequired = strlen(practiceJam3_staticState.virtualCwd) + 1 + strlen(assetPath) + 1;
    // allocate temporarily
    char* file = arena_alloc(&practiceJam3_staticState.tickArena, filePathSizeRequired);
    SDL_snprintf(file, filePathSizeRequired, "%s/%s", practiceJam3_staticState.virtualCwd, assetPath);
    stbi_uc* imageResult = stbi_load(file, &imageWidth, &imageHeight, &fileChannels, 4);

    if(!imageResult) {
        SDL_Log("Could not load character0.png: %s", stbi_failure_reason());
        SDL_Log("cwd: %s", SDL_GetCurrentDirectory());
        return false;
    }

    // SDL3 is big dumb and treats 'rgba' as big endian RGBA, but what we have is the actual bytes of R, G, B, and A in order.
    // Something about accounting for endianness or whatever - for now, just swap every single one of them around in-place.
    // TODO: is this correct on actual big-endian systems? x86 is little endian, but big endian systems do exist I think.
    uint32_t* imageResultInts = (uint32_t*)imageResult;
    for(size_t i=0; i<(unsigned int)imageWidth*(unsigned int)imageHeight; ++i) {
        uint32_t vo = imageResultInts[i];
        imageResultInts[i] = ((vo & 0xFF000000) >> 24) | ((vo & 0x00FF0000) >> 8) | ((vo & 0x0000FF00) << 8) | ((vo & 0x000000FF) << 24);
    }

    // TODO: probably free the surface and the pixel data
    SDL_Surface* imageSurface = SDL_CreateSurfaceFrom(imageWidth, imageHeight, SDL_PIXELFORMAT_RGBA8888, imageResult, imageWidth*4);

    return SDL_CreateTextureFromSurface(this->renderer, imageSurface);
}

static void getCameraTransform(PracticeJam3RenderState* this, mat4 dest, float interpolator) {
    // We want to move objects such that an object at cameraX, cameraY is at windowWidth/2, windowHeight/2
    // And so that the furthest extent of the screen (whether that is the width or the height) shows [cameraX or cameraY] += cameraRadius in world space
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSizeInPixels(this->window, &windowWidth, &windowHeight);
    unsigned int windowSize = (unsigned int)SDL_max(windowWidth, windowHeight);

    float cameraX = rqlerp(this->cameraCenterX, this->cameraCenterXLast, interpolator);
    float cameraY = rqlerp(this->cameraCenterY, this->cameraCenterYLast, interpolator);
    float cameraRadius = rqlerp(this->cameraRadius, this->cameraRadiusLast, interpolator);
    glmc_mat4_identity(dest);
    vec4 screenTranslation = {(float)windowWidth/2, (float)windowHeight/2, 0, 0};
    glmc_translate(dest, screenTranslation);
    vec4 cameraScale = {(float)windowSize / (2 * cameraRadius), (float)windowSize / (2 * cameraRadius), 1, 1};
    glmc_scale(dest, cameraScale);
    vec4 cameraTranslation = {-cameraX, -cameraY, 0, 0};
    glmc_translate(dest, cameraTranslation);
}

// Note: vertices are in top-left, top-right, bottom-left, bottom-right order and are expected to be convex
// Note: the vertex data is overriden for efficiency reasons
// Texture may be null to specify no texture
static void drawTextureQuad(PracticeJam3RenderState* this, mat4 transform, SDL_Vertex vertices[4], SDL_Texture* texture) {
    // pre-defined memory layouts ftw
    glmc_mat4_mulv3(transform, (float*)&vertices[0].position, 1, (float*)&vertices[0].position);
    glmc_mat4_mulv3(transform, (float*)&vertices[1].position, 1, (float*)&vertices[1].position);
    glmc_mat4_mulv3(transform, (float*)&vertices[2].position, 1, (float*)&vertices[2].position);
    glmc_mat4_mulv3(transform, (float*)&vertices[3].position, 1, (float*)&vertices[3].position);
    int indices[6] = {
        0, 1, 2,
        1, 3, 2,
    };
    // Ew gross reuploading vertex data every frame...
    // It's fast enough, who cares.
    SDL_RenderGeometry(this->renderer, texture, vertices, 4, indices, 6);
}

bool practiceJam3_render_init(PracticeJam3State* state) {
    state->render = arena_alloc(&state->permArena, sizeof(PracticeJam3RenderState));
    PracticeJam3RenderState* this = state->render;
    *this = (PracticeJam3RenderState){ 0 };
    this->cameraRadius = 8;

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

    this->characterTexture = loadTexture(this, "asset/character0.png");
    SDL_SetTextureScaleMode(this->characterTexture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(this->characterTexture,SDL_BLENDMODE_BLEND);
    return true;
}

bool practiceJam3_render_frame(PracticeJam3State* state) {
    PracticeJam3RenderState* this = state->render;

    // Value used to interpolate the current state and the last state
    // So if the framerate is higher than the step rate, it still looks smooth
    // 0 -> last state, 1 -> current state
    // This effectively a measurement of how far into the step we are
    float interpolator = (float)(state->times.timeNsFrame - state->times.timeNsStep + nsPerStep) / (float)nsPerStep;
    // For when unclamped interpolation might look weird
    float interpolatorClamp = SDL_clamp(interpolator, 0, 1);

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

    mat4 worldToCameraSpace;
    getCameraTransform(this, worldToCameraSpace, interpolatorClamp);

    SDL_Vertex sillyBoxVertices[4] = {
        (SDL_Vertex){.position={0, 0}, .color={1, 1, 1, 1}, .tex_coord={0, 0}},
        (SDL_Vertex){.position={1, 0}, .color={1, 1, 1, 1}, .tex_coord={1, 0}},
        (SDL_Vertex){.position={0, 1}, .color={1, 1, 1, 1}, .tex_coord={0, 1}},
        (SDL_Vertex){.position={1, 1}, .color={1, 1, 1, 1}, .tex_coord={1, 1}},
    };
    drawTextureQuad(this, worldToCameraSpace, sillyBoxVertices, NULL);

    float playerX = rqlerp(state->gameState->playerX, state->gameState->playerXLast, interpolatorClamp);
    float playerY = rqlerp(state->gameState->playerY, state->gameState->playerYLast, interpolatorClamp);
    SDL_Vertex playerVertices[4] = {
        (SDL_Vertex){.position={playerX, playerY}, .color={1, 1, 1, 1}, .tex_coord={0, 0}},
        (SDL_Vertex){.position={playerX+1, playerY}, .color={1, 1, 1, 1}, .tex_coord={1, 0}},
        (SDL_Vertex){.position={playerX, playerY+1}, .color={1, 1, 1, 1}, .tex_coord={0, 1}},
        (SDL_Vertex){.position={playerX+1, playerY+1}, .color={1, 1, 1, 1}, .tex_coord={1, 1}},
    };
    drawTextureQuad(this, worldToCameraSpace, playerVertices, this->characterTexture);
    SDL_RenderPresent(this->renderer);

    return true;
}


bool practiceJam3_render_step(PracticeJam3State* state) {
    PracticeJam3RenderState* this = state->render;
    this->cameraCenterXLast = this->cameraCenterX;
    this->cameraCenterYLast = this->cameraCenterY;
    this->cameraRadiusLast = this->cameraRadius;
    this->cameraCenterX = state->gameState->playerX;
    this->cameraCenterY = state->gameState->playerY;
    return true;
}
