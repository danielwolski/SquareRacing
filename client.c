#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include "player.h"
#include "track.h"
#include "graphics.h"
#include "constants.h"

#define MAX_PLAYERS 3

typedef struct {
    int id;
    int x;
    int y;
} PlayerData;

SDL_Surface* trackSurface;

int playerID;

int main() {

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

    SDL_Texture* trackTexture = loadTexture("track_client.png", renderer);

    if (trackTexture == NULL) {
        printf("Error loading track texture: %s\n", SDL_GetError());
        // Handle error or exit the program
    }
    
      trackSurface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

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
    
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }


    // Odbieramy nasze unikalne ID od serwera
    if (recv(clientSocket, &playerID, sizeof(int), 0) <= 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    printf("Jestem %d\n", playerID);
        PlayerData playersData[MAX_PLAYERS];
    
    SDL_Event e;
    bool running = true;
    while (running) {
    
        while (SDL_PollEvent(&e) != 0) {
          if (e.type == SDL_QUIT) {
              running = false;
          }
        }
    
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        SDL_RenderCopy(renderer, trackTexture, NULL, NULL);
        
        int received = recv(clientSocket, &playersData, sizeof(playersData), 0);
        if (received <= 0) {
            perror("recv");
            break;
        }

        printf("Dane graczy: ");
        for (int i = 0; i < MAX_PLAYERS; i++) {
            printf("(Gracz %d, x: %d, y: %d) | ", playersData[i].id, playersData[i].x, playersData[i].y);
        }
        printf("\n");
        
        for (int i = 0; i < MAX_PLAYERS; i++) {
          DrawPlayer(renderer,playersData[i].x, playersData[i].y,playersData[i].id);
        }
        DrawPlayer(renderer,25,25,playerID);
        
        SDL_RenderPresent(renderer);
    }

    close(clientSocket);
    return 0;
}

