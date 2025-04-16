// This is the "root file" where everything happens
// All subsystems are initialized here one way or another

#include "SDL3/SDL_init.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "main.h"
#define CELESTIAL_RENDER_PRIV
#include "render.h"

// Implementations of symbols in main.h

PracticeJam3State practiceJam3_staticState;

// Implementations of SDL main functions
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    practiceJam3_staticState = (PracticeJam3State){ 0 };

    // TODO: autogen version macro
    SDL_SetAppMetadata("PracticeJam3 game", "alpha-0.0.0", "com.bluesillybeard.practiceJam3game");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(!practiceJam3_render_init(&practiceJam3_staticState)) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        // TODO: send events to subsystems instead
        practiceJam3_staticState.render->closing = true;
    }
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_Log("Window resized to %i, %i", event->window.data1, event->window.data2);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    // TODO: ask subsystems if they want to close the app
    if(practiceJam3_staticState.render->closing) {
        return SDL_APP_SUCCESS;
    }

    if(!practiceJam3_render_frame(&practiceJam3_staticState)) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}
