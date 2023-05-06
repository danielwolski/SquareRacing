#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "player.h"
#include "track.h"
#include "graphics.h"
#include "constants.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Racing Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface* trackSurface = SDL_LoadBMP("track.bmp");
    if (!trackSurface) {
        printf("Unable to load track image! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    Player* player = player_create(START_X_POS, START_Y_POS);
    track_init();
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        player_update_direction(player);
        player_update_position(player, trackSurface);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        DrawPlayer(renderer, player->x, player->y);
        DrawLockStatusCircle(renderer, is_corridor_locked, 0);
        DrawLockStatusCircle(renderer, is_pitstop_locked, 1);

        SDL_RenderPresent(renderer);
    }

    player_destroy(player);
    SDL_FreeSurface(trackSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}