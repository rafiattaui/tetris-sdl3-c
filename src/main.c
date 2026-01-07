#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_timer.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BLOCK_SIZE 30 // 30 pixels per block
#define BOARD_WIDTH 10 // 10 blocks wide
#define BOARD_HEIGHT 20 // 20 blocks high

static SDL_Window* gWindow = NULL;
static SDL_Renderer* gRenderer = NULL;

const Uint64 NS_PER_SECOND = 1000000000;
const Uint64 FRAME_TARGET_NS = NS_PER_SECOND / 60;

int blocks[BOARD_HEIGHT][BOARD_WIDTH] = {0}; // 0 = no block, 1-7 blocks

struct Piece {
    int shape[4][4]; // 4x4 grid for the piece shape
    int x; // x position on the board
    int y; // y position on the board
};

struct Piece current_piece; // store current piece

SDL_Color colors[] = {
    {255, 0, 0, 255},    // Red
    {0, 255, 0, 255},    // Green
    {0, 0, 255, 255},    // Blue
    {255, 255, 0, 255},  // Yellow
    {255, 165, 0, 255},  // Orange
    {128, 0, 128, 255},  // Purple
    {0, 255, 255, 255}   // Cyan
};

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

            SDL_SetRenderDrawColorFloat(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &block);
        }
        }
    }
}

bool checkCollisions(int x, int y, struct Piece *piece)
{
    for (int row = 0; row < 4; row++){
        for (int col = 0; col < 4; col++){
            if (piece->shape[row][col]){
                int boardX = x + col;
                int boardY = y + row;

                // Check boundaries
                if (boardX < 0 || boardX >= BOARD_WIDTH || boardY < 0 || boardY >= BOARD_HEIGHT){
                    return true; // Collision detected
                }

                // Check existing blocks
                if (blocks[boardY][boardX]){
                    return true; // Collision detected
                }
            }
        }
    }
    return 0; // No collision
}

void spawn_new_piece()
{   // TODO - implement random piece generation
    // Example: spawn a simple square piece
    current_piece.shape[0][0] = 1;
    current_piece.shape[0][1] = 1;
    current_piece.shape[1][0] = 1;
    current_piece.shape[1][1] = 1;
    current_piece.x = 0;
    current_piece.y = 0;
}

void moveAndPlace(int deltaX, int deltaY)
{
    int nextX = current_piece.x + deltaX;
    int nextY = current_piece.y + deltaY;

    // Check for collisions
    bool collision = checkCollisions(nextX, nextY, &current_piece);

    if (collision){
        if (deltaY > 0){ // If moving down and collision occurs, place the piece
            for (int row = 0; row < 4; row++){
                for (int col = 0; col < 4; col++){
                    if (current_piece.shape[row][col]){
                        int boardX = current_piece.x + col;
                        int boardY = current_piece.y + row;
                        blocks[boardY][boardX] = 1; // Mark block on the board
                    }
                }
            }
            spawn_new_piece(); // Spawn a new piece after placing
        }
    } else {
        current_piece.x = nextX;
        current_piece.y = nextY;
    }
}

void drawCurrPiece(SDL_Renderer *renderer)
{
    for (int row = 0; row < 4; row++){
        for (int col = 0; col < 4; col++){
            if (current_piece.shape[row][col]){
                SDL_FRect block = {
                    .x = (float)(current_piece.x + col) * BLOCK_SIZE,
                    .y = (float)(current_piece.y + row) * BLOCK_SIZE,
                    .w = (float) BLOCK_SIZE,
                    .h = (float) BLOCK_SIZE
                };

                SDL_SetRenderDrawColorFloat(renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &block);
            }
        }
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

    SDL_SetRenderLogicalPresentation(gRenderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT){
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    int frameStart = SDL_GetTicks();

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    /* clear the window to the draw color. */
    SDL_RenderClear(gRenderer);

    draw_grid(gRenderer);
    draw_pieces(gRenderer);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(gRenderer);

    int frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < FRAME_TARGET_NS / 1000000){
        SDL_Delay((FRAME_TARGET_NS / 1000000) - frameTime);
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{

}



