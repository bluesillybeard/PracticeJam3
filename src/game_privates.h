#pragma once
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_rect.h"

// This struct exists in case I want to add more types of colliders
typedef struct _GameCollider {
    SDL_FRect AABB;
} GameCollider;

typedef struct _GameActor {
    // Position and size in world space
    float xPos;
    float yPos;
    float xSize;
    float ySize;
    // layer to render in
    int layer;
    // the actor's name - assumed to be allocated statically
    char* name;
    // the actor's texture file
    char* texture;
} GameActor;

typedef struct _GameSceneAction {
    // The amount of time to wait until playing this action if no user input is supplied
    int64_t nsDelay;
    // The minimum amount of delay to wait until playing this action  if user input is supplied
    int64_t nsDelayMin;
    enum _GameSceneActionType {
        // literally just wait and do nothing
        GameSceneActionType_none,
        GameSceneActionType_lockPlayer,
        GameSceneActionType_unlockPlayer,
        // Spawn an actor into the world
        GameSceneActionType_spawnActor,
        // Remove an actor from existence
        GameSceneActionType_deleteActor,
        // Move an actor to a location in the world
        GameSceneActionType_moveActor,
        GameSceneActionType_speak,
    } actionType;

    union _GameSceneActionData {
        GameActor spawnActor;
        struct _GameSceneActionDataDeleteActor {
            // name of the actor to delete
            char* name;
        } deleteActor;
        struct _GameSceneActionDataMoveActor {
            // name of the actor to move
            char* name;
            // where to move the actor
            float xPos;
            float yPos;
            // the period of time over which to move the actor in nanoseconds
            int64_t nsPeriod;
        } moveActor;
        struct _GameSceneActionDataSpeak {
            char* string;
            // how long to keep the text up
            int64_t nsTime;
        } speak;
    } data;

} GameSceneAction;

typedef struct _GameScene {
    char* backdrop;
    SDL_FRect backdropDst;
    float playerStartX;
    float playerStartY;
    GameCollider const* colliders;
    size_t numColliders;
} GameScene;

struct _PracticeJam3GameState {
    float playerX;
    float playerY;
    float playerXLast;
    float playerYLast;
    bool upControlState;
    bool downControlState;
    bool leftControlState;
    bool rightControlState;
    SDL_Texture* characterTexture;
    SDL_Texture* scene1Texture;
    bool showHitBoxes;
    // If the player's movement is disabled
    bool playerLocked;
};
