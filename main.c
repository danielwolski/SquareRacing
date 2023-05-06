#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "player.h"
#include "track.h"
#include "graphics.h"
#include "constants.h"

const Color CORRIDOR_IN_COLOR = { 0, 0, 255 };
const Color CORRIDOR_OUT_COLOR = { 0, 255, 0 };
const Color PITSTOP_IN_COLOR = { 0, 255, 255 };
const Color PITSTOP_OUT_COLOR = { 255, 200, 0 };
const Color DONT_ENTER_COLOR = { 255, 255, 255 };

int is_corridor_locked = 0;
int is_pitstop_locked = 0;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Racing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Error creating window: %s\n", SDL_GetError());
        // Handle error or exit the program
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        // Handle error or exit the program
    }

    SDL_Texture* trackTexture = loadTexture("track.png", renderer);

    if (trackTexture == NULL) {
        printf("Error loading track texture: %s\n", SDL_GetError());
        // Handle error or exit the program
    }

    SDL_Surface* trackSurface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    if (trackSurface == NULL) {
        printf("Error creating track surface: %s\n", SDL_GetError());
        // Handle error or exit the program
    }
     
    SDL_Texture* target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_SetRenderTarget(renderer, target);
    SDL_RenderCopy(renderer, trackTexture, NULL, NULL);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, trackSurface->pixels, trackSurface->pitch);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_DestroyTexture(target);

    Player* player = player_create(START_X_POS, START_Y_POS);
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        player_update_position(player, trackSurface);
        
        printf("current point %d\n", player->current_track_point);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        SDL_RenderCopy(renderer, trackTexture, NULL, NULL);

        DrawPlayer(renderer, player->x, player->y);
        
        for (int i = 0; i < NUM_TRACK_POINTS; i++) {
        DrawPoint(renderer, track_points[i]);
        }
        
        DrawLockStatusCircle(renderer, is_corridor_locked, -33);
        DrawLockStatusCircle(renderer, is_pitstop_locked, 20);

        SDL_RenderPresent(renderer);
        
        SDL_Delay(3);
    }

    player_destroy(player);
    
    SDL_FreeSurface(trackSurface);
    SDL_DestroyTexture(trackTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
