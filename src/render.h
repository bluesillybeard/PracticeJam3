#pragma once
#include <stdbool.h>
#include "SDL3/SDL_render.h"
#include "main.h"

bool practiceJam3_render_init(PracticeJam3State* state);

bool practiceJam3_render_frame(PracticeJam3State* state);

void practiceJam3_render_setCamera(PracticeJam3State* state, float centerX, float centerY, float radius);

bool practiceJam3_render_step(PracticeJam3State* state);

SDL_Texture* practiceJam3_render_loadTexture(PracticeJam3State* state, char* assetPath);

bool practiceJam3_render_sprite(PracticeJam3State* state, float x, float y, float w, float h, SDL_Texture* texture, float tintRed, float tintGreen, float tintBlue, float tintAlpha, int layer);

// Super basic text rendering function
// X and Y are where the baseline goes, meaning the font itself may be rendered to the left or below (x, y)
// See practiceJap3_measure_text for how to measure the size and offset of a text string
bool practiceJam3_render_text(PracticeJam3State* state, float x, float y, float size, float tintRed, float tintGreen, float tintBlue, float tintAlpha, char* text, int layer);

// Value used to interpolate the current state and the last state
// So if the framerate is higher than the step rate, it still looks smooth
// 0 -> last state, 1 -> current state
// This effectively a measurement of how far into the step we are
float practiceJam3_render_getInterpolator(PracticeJam3State* state);

bool practiceJam3_render_event(PracticeJam3State* state, SDL_Event const* ev);

void practiceJam3_render_quit(PracticeJam3State* state);
