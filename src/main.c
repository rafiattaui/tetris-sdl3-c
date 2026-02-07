#include "SDL3/SDL_events.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <stdbool.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string.h>

static SDL_Window* gWindow = NULL;
static SDL_Renderer* gRenderer = NULL;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BLOCK_SIZE 40 // 30 pixels per block
#define BOARD_WIDTH 10 // 10 blocks wide
#define BOARD_HEIGHT 15 // 20 blocks high
#define PIECE_WIDTH 4
#define PIECE_HEIGHT 4

int lastTime = 0, currentTime;

int blocks[BOARD_HEIGHT][BOARD_WIDTH] = {0, [14][1]=1, [14][2]=1,[13][1]=1}; // 0 = no block, 1 = block
int nextRotation[PIECE_HEIGHT][PIECE_WIDTH];

struct Block {
    int x; // x position on the board, starting from top
    int y; // y position on the board, starting from left
    int shape[PIECE_HEIGHT][PIECE_WIDTH]; // 4x4 shape matrix
};

enum collisionType {
    NONE = 0,
    WALL,
    FLOOR,
    BLOCK
};

struct Block currentBlock;
struct Block nextBlock;

int collisionState;

void draw_grid(SDL_Renderer *renderer)
{
    const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */

    for (int row = 0; row < BOARD_HEIGHT; row++){
        for (int col = 0; col < BOARD_WIDTH; col++){
            SDL_FRect cell = {
                .x = (float) col * BLOCK_SIZE,
                .y = (float) row * BLOCK_SIZE,
                .w = (float) BLOCK_SIZE,
                .h = (float) BLOCK_SIZE
            };

            SDL_RenderRect(renderer, &cell);
        }
    }
}

void draw_pieces(SDL_Renderer *renderer)
{
    for (int row = 0; row < BOARD_HEIGHT; row++){
        for (int col = 0; col < BOARD_WIDTH; col++){
            if (blocks[row][col] == 1){
            SDL_FRect block = {
                .x = (float) col * BLOCK_SIZE,
                .y = (float) row * BLOCK_SIZE,
                .w = (float) BLOCK_SIZE,
                .h = (float) BLOCK_SIZE
            };
            SDL_RenderFillRect(renderer, &block);
        }
        }
    }
}

void draw_blocks(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    for (int row = 0; row < PIECE_HEIGHT; row++){
        for (int col = 0; col < PIECE_WIDTH; col++){
            if (currentBlock.shape[row][col] == 1){ // these block coordinates are still in block space, need to be transformed to grid-space
                SDL_FRect block = {
                    .x = (float) (currentBlock.x + col) * BLOCK_SIZE,
                    .y = (float) (currentBlock.y + row) * BLOCK_SIZE,
                    .w = (float) BLOCK_SIZE,
                    .h = (float) BLOCK_SIZE
                };
                SDL_RenderFillRect(renderer, &block);
            }
        }
    }
}

int check_collision(struct Block *piece, int newX, int newY){
    for (int row = 0; row < PIECE_HEIGHT; row++){
        for (int col = 0; col < PIECE_WIDTH; col++){
            if (piece->shape[row][col] != 0){
                int targetX = newX + col;
                int targetY = newY + row;

                if (targetX < 0 || targetX >= BOARD_WIDTH){
                    return WALL;
                }

                if (targetY < 0 || targetY >= BOARD_HEIGHT){
                    return FLOOR;
                }

                if (blocks[targetY][targetX] != 0){
                    return BLOCK;
                }
            }
        }
    }
    return NONE;
}

// TODO - Needs to swap arrays to pointer logic.
bool rotate_piece(struct Block *piece){

    for (int row = 0; row < PIECE_HEIGHT; row++){
        for (int col = 0; col < PIECE_WIDTH; row++){
            nextRotation[col][PIECE_HEIGHT - 1 - row] = piece->shape[row][col];
        }
    }
    return true;
}   

void map_piece(struct Block *piece){
    for (int row = 0; row < PIECE_HEIGHT; row++){
        for (int col = 0; col < PIECE_WIDTH; col++){
            if (piece->shape[row][col] != 0){
                int targetX = piece->x + col;
                int targetY = piece->y + row;

                blocks[targetY][targetX] = piece->shape[row][col];
            }
        }
    }
    collisionState = 0;
}

void move_down(struct Block *piece){
    int newX = currentBlock.x;
    int newY = currentBlock.y + 1;

    collisionState = check_collision(piece, newX, newY);
    if (collisionState == NONE){
        currentBlock.x = newX;
        currentBlock.y = newY;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Tetris in C using SDL3", "1.0", "com.rafiattaa.tetris-sdl3-c");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Tetris in C using SDL3", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer)){
        SDL_Log("Couldn't create window: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(gRenderer, SCREEN_HEIGHT, SCREEN_WIDTH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    currentBlock.shape[0][0] = 1;
    currentBlock.shape[0][1] = 1;
    currentBlock.shape[0][2] = 1;
    currentBlock.shape[1][0] = 1;
    currentBlock.x = 0;
    currentBlock.y = 0;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT){
        return SDL_APP_SUCCESS;
    }

    else if (event->type == SDL_EVENT_KEY_UP){
        if (event->key.scancode == SDL_SCANCODE_D){
            int newX = currentBlock.x + 1;
            int newY = currentBlock.y;
            collisionState = check_collision(&currentBlock, newX, newY);
            if (collisionState == NONE){
                currentBlock.x = newX;
                currentBlock.y = newY;
            }
        }

        else if (event->key.scancode == SDL_SCANCODE_A){
            int newX = currentBlock.x - 1;
            int newY = currentBlock.y;
            collisionState = check_collision(&currentBlock, newX, newY);
            if (collisionState == NONE){
                currentBlock.x = newX;
                currentBlock.y = newY;
            }
        }

        else if (event->key.scancode == SDL_SCANCODE_S){
            move_down(&currentBlock);
        }

        else if (event->key.scancode == SDL_SCANCODE_R){
            int nextRotation[PIECE_HEIGHT][PIECE_WIDTH] = {0};

            bool isRotatable = rotate_piece(&currentBlock);
            if (isRotatable){
                memcpy(&currentBlock.shape, &nextRotation, sizeof(currentBlock.shape));
            }
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    currentTime = SDL_GetTicks();
    if (currentTime > lastTime + 1000){
        move_down(&currentBlock);
        lastTime = currentTime;
    }

    SDL_Log("%d\n", collisionState);

    if (collisionState == 3 || collisionState == 2){ // 3 means block collision
        map_piece(&currentBlock);
        currentBlock.x = 0;
        currentBlock.y = 0;
    }

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    /* clear the window to the draw color. */
    SDL_RenderClear(gRenderer);

    draw_grid(gRenderer);
    draw_pieces(gRenderer);
    draw_blocks(gRenderer);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(gRenderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{

}



