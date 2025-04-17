// This is the "root file" where everything happens
// All subsystems are initialized here one way or another

#include "arena.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "main.h"

#define RENDER_PRIV
#include "render.h"
#include "game.h"
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

    // TODO: auto generate version macro
    SDL_SetAppMetadata("PracticeJam3 game", "alpha-0.0.0", "com.bluesillybeard.practiceJam3game");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Due to _reasons_, the binary is in a folder adjacent to the assets folder
    // Normally I would have the binary next to the assets folder,
    // But cmake is being annoying and I don't want to deal with it to be honest
    // So for anything other than emscripten, create a virtual cwd that points to the folder right outside the one our exe is in
    // TODO: it's probably best to do a minecraft and make all of the different folders configurable
    // Not for this game jam though, for now it's good enough
    #if !defined(__EMSCRIPTEN__)
    char* realCwd = SDL_GetCurrentDirectory();
    size_t cwdLenToCheck = strlen(realCwd);
    if(realCwd[cwdLenToCheck-1] == '/' || realCwd[cwdLenToCheck-1] == '\\') {
        cwdLenToCheck--;
    }
    size_t indexOfLastFolderSeparator = 0;
    for(size_t i=0; i<cwdLenToCheck; ++i) {
        if(realCwd[i] == '/') {
            indexOfLastFolderSeparator = i;
        } else if(realCwd[i] == '\\') {
            // Windows be like:
            indexOfLastFolderSeparator = i;
        }
    }
    // I could really use a proper string management library (no, snprintf and scanf doesn't count)
    char* virtualCwdAllocation = arena_alloc(&_state.permArena, indexOfLastFolderSeparator+1);
    SDL_memcpy(virtualCwdAllocation, realCwd, indexOfLastFolderSeparator);
    virtualCwdAllocation[indexOfLastFolderSeparator] = 0;
    _state.virtualCwd = virtualCwdAllocation;
    #else
    _state.virtualCwd = "";
    #endif

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
    practiceJam3_game_event(&_state, event);
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

        arena_reset(&_state.tickArena);
    }

    if(!practiceJam3_game_frame(&_state)) {
        return SDL_APP_FAILURE;
    }

    if(!practiceJam3_render_frame(&_state)) {
        return SDL_APP_FAILURE;
    }

    arena_reset(&_state.frameArena);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;
}
