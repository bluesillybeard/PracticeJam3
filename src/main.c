// This is the "root file" where everything happens
// All subsystems are initialized here one way or another

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_timer.h"
#include "game.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "main.h"
#define RENDER_PRIV
#include "render.h"

// Implementations of symbols in main.h

PracticeJam3State practiceJam3_staticState;
#define _state practiceJam3_staticState

// Implementations of SDL main functions
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)appstate;
    (void)argc;
    (void)argv;
    _state = (PracticeJam3State){ 0 };

    // TODO: autogen version macro
    SDL_SetAppMetadata("PracticeJam3 game", "alpha-0.0.0", "com.bluesillybeard.practiceJam3game");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(!practiceJam3_render_init(&_state)) {
        return SDL_APP_FAILURE;
    }

    if(!practiceJam3_game_init(&_state)) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;
    if (event->type == SDL_EVENT_QUIT) {
        // TODO: send events to subsystems instead
        _state.render->closing = true;
    }
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_Log("Window resized to %i, %i", event->window.data1, event->window.data2);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;
    // TODO: ask subsystems if they want to close the app
    if(_state.render->closing) {
        return SDL_APP_SUCCESS;
    }
    
    _state.times.timeNsLastFrame = _state.times.timeNsFrame;
    _state.times.timeNsFrame = (int64_t)SDL_GetTicksNS();

    // In the event of lag, skip forward a bit
    // the lagOffset is something that may be worth adjusting if things behave strangely during lag
    int64_t lagOffset = nsPerStep*3;
    if((_state.times.timeNsStep) < (_state.times.timeNsFrame - lagOffset)) {
        _state.times.timeNsStep = _state.times.timeNsFrame - lagOffset;
    }

    // Run steps until we've achieved a rate of 60 steps/sec
    while(_state.times.timeNsStep < _state.times.timeNsFrame) {
        _state.times.timeNsLastStep = _state.times.timeNsStep;
        _state.times.timeNsStep += nsPerStep;
        _state.times.timeNsGame += nsPerStep;
        
        if(!practiceJam3_game_step(&_state)) {
            return SDL_APP_FAILURE;
        }

        if(!practiceJam3_render_step(&_state)) {
            return SDL_APP_FAILURE;
        }
    }

    if(!practiceJam3_game_frame(&_state)) {
        return SDL_APP_FAILURE;
    }

    if(!practiceJam3_render_frame(&_state)) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;
}
