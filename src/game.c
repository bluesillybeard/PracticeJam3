#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "cglm/util.h"
#include "main.h"
#include "render.h"
#include <pulse/context.h>
#include <unistd.h>
#include "game.h"
#include <stdbool.h>

#include "game_privates.h"

#include "scenes.h"


static inline float gameCross2DZ(float ax, float ay, float bx, float by) {
    return ax*by - ay*bx;
}

static inline float gameSegmentsIntersectTimev(float xa0, float ya0, float xad, float yad, float xb0, float yb0, float xbd, float ybd) {
    return gameCross2DZ(xb0 - xa0, yb0 - ya0, xbd, ybd) / gameCross2DZ(xad, yad, xbd, ybd);
}

static inline float gameSegmentsIntersectTimep(float xa0, float ya0, float xa1, float ya1, float xb0, float yb0, float xb1, float yb1) {
    return gameSegmentsIntersectTimev(xa0, ya0, xa1-xa0, ya1-ya0, xb0, yb0, xb1-xb0, yb1-yb0);
}

// A move-and-slide type AABB collision detector
// 0. -> no collision
// 1. -> "top" of a
// 2. -> "bottom" of a
// 3. -> left of a
// 4. -> right of a
// Can also be seen as an offset bitfield:
// - sign = (collide()-1)&1
// - vertical = (collide()-1)&2
// 
// This gives mathematically correct but generally wrong/confusing results when the bodies A and B overlap
static int gameCollide(GameCollider a, float adx, float ady, GameCollider b, float bdx, float bdy, float* t) {
    // This is based on this Desmos graph: https://www.desmos.com/calculator/pijv3shonk
    // I created that graph based on something else.
    // TODO: implement the optimizations mentioned in the Desmos graph

    // Short parameters are OK, but I just hate short variable names in an algorithm like this.
    // Also, having to go through AABB for every member is just kinda annoying.
    // Finally: this makes the names identical to the ones in the Desmos graph, which just makes it easier to copy everything.
    float const aPosX = a.AABB.x;
    float const aPosY = a.AABB.y;
    float const aSizeX = a.AABB.w;
    float const aSizeY = a.AABB.h;
    float const aMoveX = adx;
    float const aMoveY = ady;

    float const bPosX = b.AABB.x;
    float const bPosY = b.AABB.y;
    float const bSizeX = b.AABB.w;
    float const bSizeY = b.AABB.h;
    float const bMoveX = bdx;
    float const bMoveY = bdy;

    float const moveRelX = aMoveX - bMoveX;
    float const moveRelY = aMoveY - bMoveY;

    // Calculate translated minkowski AABB

    float const mkPosX = aPosX - bPosX - bSizeX + moveRelX;
    float const mkPosY = aPosY - bPosY - bSizeY + moveRelY;
    float const mkSizeX = aSizeX + bSizeX;
    float const mkSizeY = aSizeY + bSizeY;

    // Get all of the intersection times
    // note: top and bottom are reversed to match the names in the Desmos graph
    // TODO: flip top and bottom back so it makes sense again, OR name them independent of orientation (like +x, -x, etc)

    float const topIntersA = 1 - gameSegmentsIntersectTimep(0, 0, moveRelX, moveRelY, mkPosX+mkSizeX, mkPosY+mkSizeY, mkPosX, mkPosY+mkSizeY);
    float const topIntersB = 1 - gameSegmentsIntersectTimep(mkPosX+mkSizeX, mkPosY+mkSizeY, mkPosX, mkPosY+mkSizeY, 0, 0, moveRelX, moveRelY);
    float const bottomIntersA = 1 - gameSegmentsIntersectTimep(0, 0, moveRelX, moveRelY, mkPosX+mkSizeX, mkPosY, mkPosX, mkPosY);
    float const bottomIntersB = 1 - gameSegmentsIntersectTimep(mkPosX+mkSizeX, mkPosY, mkPosX, mkPosY, 0, 0, moveRelX, moveRelY);
    float const leftIntersA = 1 - gameSegmentsIntersectTimep(0, 0, moveRelX, moveRelY, mkPosX, mkPosY+mkSizeY, mkPosX, mkPosY);
    float const leftIntersB = 1 - gameSegmentsIntersectTimep(mkPosX, mkPosY+mkSizeY, mkPosX, mkPosY, 0, 0, moveRelX, moveRelY);
    float const rightIntersA = 1 - gameSegmentsIntersectTimep(0, 0, moveRelX, moveRelY, mkPosX+mkSizeX, mkPosY+mkSizeY, mkPosX+mkSizeX, mkPosY);
    float const rightIntersB = 1 - gameSegmentsIntersectTimep(mkPosX+mkSizeX, mkPosY+mkSizeY, mkPosX+mkSizeX, mkPosY, 0, 0, moveRelX, moveRelY);

    float closestCollisionTime = 1.0f/0.0f; // TODO: infinity? Should probably refactor to avoid it entirely
    int closestCollisionFace = 0;

    if(topIntersA > 0 && topIntersA < 1 && topIntersB > 0 && topIntersB < 1 && closestCollisionTime > topIntersA) {
        closestCollisionTime = topIntersA;
        closestCollisionFace = 1;
    }
    if(bottomIntersA > 0 && bottomIntersA < 1 && bottomIntersB > 0 && bottomIntersB < 1 && closestCollisionTime > bottomIntersA) {
        closestCollisionTime = bottomIntersA;
        closestCollisionFace = 2;
    }
    if(leftIntersA > 0 && leftIntersA < 1 && leftIntersB > 0 && leftIntersB < 1 && closestCollisionTime > leftIntersA) {
        closestCollisionTime = leftIntersA;
        closestCollisionFace = 3;
    }
    if(rightIntersA > 0 && rightIntersA < 1 && rightIntersB > 0 && rightIntersB < 1 && closestCollisionTime > rightIntersA) {
        closestCollisionTime = rightIntersA;
        closestCollisionFace = 4;
    }

    *t = closestCollisionTime;
    return closestCollisionFace;
}

bool practiceJam3_game_init(PracticeJam3State* state) {
    state->gameState = arena_alloc(&state->permArena, sizeof(PracticeJam3GameState));
    PracticeJam3GameState* this = state->gameState;
    *this = (PracticeJam3GameState){0};

    this->characterTexture = practiceJam3_render_loadTexture(state, "asset/character0.png");
    SDL_SetTextureScaleMode(this->characterTexture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(this->characterTexture,SDL_BLENDMODE_BLEND);

    this->scene1Texture = practiceJam3_render_loadTexture(state, scene1.backdrop);
    SDL_SetTextureScaleMode(this->scene1Texture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(this->scene1Texture,SDL_BLENDMODE_BLEND);

    this->playerX = scene1.playerStartX;
    this->playerY = scene1.playerStartY;
    return true;
}

bool practiceJam3_game_frame(PracticeJam3State* state) {
    PracticeJam3GameState* this = state->gameState;
    float interpolator = practiceJam3_render_getInterpolator(state);
    float playerX = glm_lerpc(this->playerXLast, this->playerX, interpolator);
    float playerY = glm_lerpc(this->playerYLast, this->playerY, interpolator);
    if(!practiceJam3_render_sprite(state, playerX, playerY, 1, 1, this->characterTexture, 1, 1, 1, 1, 0)) {
        return false;
    }
    if(!practiceJam3_render_text(state, 10, 10, 3, 1, 1, 1, 1, "A", 10)){
        return false;
    }

    practiceJam3_render_sprite(state, scene1.backdropDst.x, scene1.backdropDst.y, scene1.backdropDst.w, scene1.backdropDst.h, this->scene1Texture, 1, 1, 1, 1, -20);

    if(this->showHitBoxes){
        for(size_t i=0; i<scene1.numColliders; ++i) {
            GameCollider col = scene1.colliders[i];
            practiceJam3_render_sprite(state, col.AABB.x, col.AABB.y, col.AABB.w, col.AABB.h, NULL, 0.5f, 0.8f, 0.3f, 0.5f, -10);
        }
    }
    return true;
}

bool practiceJam3_game_event(PracticeJam3State* state, SDL_Event const* ev) {
    PracticeJam3GameState* this = state->gameState;
    // Gotta love fall-through by default
    switch (ev->type) {
        case SDL_EVENT_KEY_DOWN: {
            switch (ev->key.key) {
                case SDLK_W: this->upControlState = true; break;
                case SDLK_A: this->leftControlState = true; break;
                case SDLK_S: this->downControlState = true; break;
                case SDLK_D: this->rightControlState = true; break;
                case SDLK_PERIOD: this->showHitBoxes = true; break;
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            switch (ev->key.key) {
                case SDLK_W: this->upControlState = false; break;
                case SDLK_A: this->leftControlState = false; break;
                case SDLK_S: this->downControlState = false; break;
                case SDLK_D: this->rightControlState = false; break;
                case SDLK_PERIOD: this->showHitBoxes = false; break;
            }
            break;
        }
    }
    return true;
}

bool practiceJam3_game_step(PracticeJam3State* state) {
    PracticeJam3GameState* this = state->gameState;
    this->playerXLast = this->playerX;
    this->playerYLast = this->playerY;
    float delta = (float)nsPerStep / (1000.0f*1000.0f*1000.0f);
    float const playerSpeed = 5;
    float moveX = 0;
    float moveY = 0;
    if(!this->playerLocked){
        if(this->upControlState) {
            moveY -= playerSpeed * delta;
        }
        if(this->downControlState) {
            moveY += playerSpeed * delta;
        }
        if(this->rightControlState) {
            moveX += playerSpeed * delta;
        }
        if(this->leftControlState) {
            moveX -= playerSpeed * delta;
        }
    }

    // Note: this is robust, but it's worth mentioning that it only checks for when the colliders cross from/to intersecting each other.
    // In other words if the objects start intersected, the collide function reports a collision for when they stop colliding
    // In practical terms, it means that if the player starts within a bounding box, they cannot leave that bounding box

    // avoid issues with precision and getting stuck by enacting a buffer
    float const collisionTimeBuffer = 1.0f / 512.0f;
    
    // Check collision against level AABB
    for(size_t index = 0; index < scene1.numColliders; ++index) {
        GameCollider col = scene1.colliders[index];
        float collisionTime;
        int collisionFace = gameCollide((GameCollider){.AABB={this->playerX, this->playerY, 1, 1}}, moveX, moveY, col, 0, 0, &collisionTime);
        collisionTime -= collisionTimeBuffer;
        if(collisionTime < 0) collisionTime = 0;
        if(collisionFace){
            switch (collisionFace) {
                case 1:
                    moveY *= collisionTime; break;
                case 2:
                    moveY *= collisionTime; break;
                case 3:
                    moveX *= collisionTime; break;
                case 4:
                    moveX *= collisionTime; break;
            }
        }
    }

    this->playerX += moveX;
    this->playerY += moveY;

    practiceJam3_render_setCamera(state, this->playerX+0.5f, this->playerY+0.5f, 20);
    return true;
}
