// This is the "root file" where everything happens
// All subsystems are initialized here one way or another

#include "SDL3/SDL_init.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "main.h"
#define CELESTIAL_RENDER_PRIV
#include "render.h"

// Implementations of symbols in main.h

CelestialState celestial_staticState;

// Implementations of SDL main functions
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    celestial_staticState = (CelestialState){ 0 };

    // TODO: autogen version macro
    SDL_SetAppMetadata("Celestial game", "alpha-0.0.0", "com.bluesillybeard.celestialgame");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(!celestial_render_init(&celestial_staticState)) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        // TODO: send events to subsystems instead
        celestial_staticState.render->closing = true;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    // TODO: ask subsystems if they want to close the app
    if(celestial_staticState.render->closing) {
        return SDL_APP_SUCCESS;
    }

    if(!celestial_render_frame(&celestial_staticState)) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}
