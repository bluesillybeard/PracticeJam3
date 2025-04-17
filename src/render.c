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
#include <time.h>

#include "arena.h"
#include "cglm/call/affine.h"
#include "cglm/call/mat4.h"
#include "cglm/types.h"
#include "stb_image.h"

#define RENDER_PRIV
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

// other extra functions
static inline float rqlerp(float a, float b, float x) {
    return a * x + b * (1-x);
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

static RenderLayer* getOrMakeRenderLayer(PracticeJam3RenderState* this, int wantedDepth) {
    size_t layerIndex = 0;
    if(this->layers) {
        // binary search for this layer
        size_t boundLow = 0;
        size_t boundHigh = this->numRenderLayers-1;
        while(true) {
            size_t const index = (boundHigh + boundLow)/2;
            int const layerDepth = this->layers[index].depth;
            if(wantedDepth == layerDepth) {
                return &this->layers[index];
            } else {
                if(wantedDepth > layerDepth) {
                    // Too low but we've hit the end of the search
                    if(boundLow == boundHigh) {
                        layerIndex = boundLow+1;
                        break;
                    }
                    boundLow = index+1;
                } else if(wantedDepth < layerDepth) {
                    // Too high but we've hit the end of the search
                    if(boundLow == boundHigh) {
                        layerIndex = boundLow;
                        break;
                    }
                    boundHigh = index;
                }
            }
        }
    }
    // No layer!
    RenderLayer newLayer = {
        .capacity = 0,
        .cmds = NULL,
        .count = 0,
        .depth = wantedDepth,
    };
    // Creating new layers should be seriously quite rare, so the slowness is probably fine
    RenderLayer* newLayers = SDL_malloc((this->numRenderLayers+1) * sizeof(RenderLayer));
    newLayers[layerIndex] = newLayer;
    if(this->layers) {
        SDL_memcpy(newLayers, this->layers, (layerIndex) * sizeof(RenderLayer));
        SDL_memcpy(newLayers+layerIndex+1, this->layers+layerIndex, (this->numRenderLayers - layerIndex) * sizeof(RenderLayer));
        SDL_free(this->layers);
    }
    this->layers = newLayers;
    this->numRenderLayers++;
    return &this->layers[layerIndex];
}

// Makes a copy of com
void renderLayer_add(RenderLayer* layer, RenderCommand const* com) {
    if(!layer->cmds) {
        layer->capacity = 8;
        layer->count = 0;
        layer->cmds = SDL_malloc(8 * sizeof(RenderCommand));
    }
    if(layer->capacity == layer->count) {
        size_t newCapcity = layer->capacity*2;
        layer->cmds = SDL_realloc(layer->cmds, newCapcity * sizeof(RenderCommand));
        layer->capacity = newCapcity;
    }
    layer->cmds[layer->count] = *com;
    layer->count++;
}

bool practiceJam3_render_init(PracticeJam3State* state) {
    state->render = arena_alloc(&state->permArena, sizeof(PracticeJam3RenderState));
    PracticeJam3RenderState* this = state->render;
    *this = (PracticeJam3RenderState){ 0 };
    this->cameraRadius = 12;

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
    return true;
}

bool practiceJam3_render_frame(PracticeJam3State* state) {
    PracticeJam3RenderState* this = state->render;

    float interpolator = practiceJam3_render_getInterpolator(state);
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
    // Also, SDL does in fact trigger the resize event upon calling this function
    if(width != realWidth || height != realHeight) {
        SDL_SetWindowSize(this->window, width, height);
    }
    #endif

    SDL_SetRenderDrawColor(this->renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(this->renderer);

    mat4 worldToCameraSpace;
    getCameraTransform(this, worldToCameraSpace, interpolatorClamp);

    for(size_t layer = 0; layer < this->numRenderLayers; layer++) {
        RenderLayer* layerObj = &this->layers[layer];
        for(size_t command=0; command<layerObj->count; command++) {
            RenderCommand* co = &layerObj->cmds[command];
            SDL_Vertex vertices[4] = {
                (SDL_Vertex){.position = {co->x      , co->y      }, .color = {co->tintRed, co->tintGreen, co->tintBlue, co->tintAlpha}, .tex_coord = {0, 0}},
                (SDL_Vertex){.position = {co->x+co->w, co->y      }, .color = {co->tintRed, co->tintGreen, co->tintBlue, co->tintAlpha}, .tex_coord = {1, 0}},
                (SDL_Vertex){.position = {co->x      , co->y+co->h}, .color = {co->tintRed, co->tintGreen, co->tintBlue, co->tintAlpha}, .tex_coord = {0, 1}},
                (SDL_Vertex){.position = {co->x+co->w, co->y+co->h}, .color = {co->tintRed, co->tintGreen, co->tintBlue, co->tintAlpha}, .tex_coord = {1, 1}},
            };
            drawTextureQuad(this, worldToCameraSpace, vertices, co->texture);
        }
        // reset the layer
        SDL_memset(layerObj->cmds, 0, layerObj->count * sizeof(RenderCommand));
        layerObj->count = 0;
    }

    SDL_RenderPresent(this->renderer);

    return true;
}

void practiceJam3_render_setCamera(PracticeJam3State* state, float centerX, float centerY, float radius) {
    PracticeJam3RenderState* this = state->render;
    this->cameraCenterXSet = centerX;
    this->cameraCenterYSet = centerY;
    this->cameraRadiusSet = radius;
}

bool practiceJam3_render_step(PracticeJam3State* state) {
    PracticeJam3RenderState* this = state->render;
    
    this->cameraCenterXLast = this->cameraCenterX;
    this->cameraCenterYLast = this->cameraCenterY;
    this->cameraRadiusLast = this->cameraRadius;

    this->cameraCenterX = this->cameraCenterXSet;
    this->cameraCenterY = this->cameraCenterYSet;
    this->cameraRadius = this->cameraRadiusSet;
    return true;
}

SDL_Texture* practiceJam3_render_loadTexture(PracticeJam3State* state, char* assetPath) {
    PracticeJam3RenderState* this = state->render;
    int imageWidth;
    int imageHeight;
    int fileChannels;

    size_t filePathSizeRequired = strlen(practiceJam3_staticState.virtualCwd) + 1 + strlen(assetPath) + 1;
    // allocate temporarily
    char* file = arena_alloc(&practiceJam3_staticState.tickArena, filePathSizeRequired);
    SDL_snprintf(file, filePathSizeRequired, "%s/%s", practiceJam3_staticState.virtualCwd, assetPath);
    stbi_uc* imageResult = stbi_load(file, &imageWidth, &imageHeight, &fileChannels, 4);

    if(!imageResult) {
        SDL_Log("Could not load %s: %s", assetPath, stbi_failure_reason());
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

    SDL_Texture* tex = SDL_CreateTextureFromSurface(this->renderer, imageSurface);

    SDL_DestroySurface(imageSurface);

    stbi_image_free(imageResult);

    return tex;
}

bool practiceJam3_render_sprite(PracticeJam3State* state, float x, float y, float w, float h, SDL_Texture* texture, float tintRed, float tintGreen, float tintBlue, float tintAlpha, int layer) {
    PracticeJam3RenderState* this = state->render;
    RenderCommand com = {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .texture = texture,
        .tintRed = tintRed,
        .tintGreen = tintGreen,
        .tintBlue = tintBlue,
        .tintAlpha = tintAlpha
    };
    RenderLayer* renderL = getOrMakeRenderLayer(this, layer);
    renderLayer_add(renderL, &com);
    return true;
}

float practiceJam3_render_getInterpolator(PracticeJam3State* state) {
    return (float)(state->times.timeNsFrame - state->times.timeNsStep + nsPerStep) / (float)nsPerStep;
}
