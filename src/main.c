#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window* gWindow = NULL;
static SDL_Renderer* gRenderer = NULL;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BLOCK_SIZE 30 // 30 pixels per block
#define BOARD_WIDTH 10 // 10 blocks wide
#define BOARD_HEIGHT 20 // 20 blocks high

int blocks[BOARD_HEIGHT][BOARD_WIDTH] = {1, 1}; // 0 = no block, 1 = block

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
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    /* clear the window to the draw color. */
    SDL_RenderClear(gRenderer);

    draw_grid(gRenderer);
    draw_pieces(gRenderer);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(gRenderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{

}



