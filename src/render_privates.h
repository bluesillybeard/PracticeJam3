#pragma once

#include "SDL3/SDL_render.h"
#include "ext/stb_truetype.h"

typedef struct _RenderCommand {
    enum _RenderCommandType{
        RenderCommandType_sprite,
        RenderCommandType_text,
    } commandType;
    union _RenderCommandData {
        struct _RenderCommandDataSprite {
            float x;
            float y;
            float w;
            float h;
            SDL_Texture* texture;
            float tintRed;
            float tintGreen;
            float tintBlue;
            float tintAlpha;
        } sprite;
        struct _RenderCommandDataText {
            float x;
            float y;
            float tintRed;
            float tintGreen;
            float tintBlue;
            float tintAlpha;
            float size;
            char* text;
        } text;
    } data;
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
    SDL_Texture* fontAtlasTexture;
    stbtt_packedchar* fontCharData;
    size_t fontCharDataNum;
    stbtt_fontinfo fontInfo;
    unsigned int fontAtlasWidth;
    unsigned int fontAtlasHeight;
};
