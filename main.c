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

const Color CORRIDOR_IN_COLOR = { 0, 0, 255 };
const Color CORRIDOR_OUT_COLOR = { 0, 255, 0 };
const Color PITSTOP_IN_COLOR = { 0, 255, 255 };
const Color PITSTOP_OUT_COLOR = { 255, 200, 0 };
const Color DONT_ENTER_COLOR = { 255, 255, 255 };

int is_corridor_locked = 0;
int is_pitstop_locked = 0;

#define FPS 60
#define FRAME_DURATION (1000 / FPS)

Uint32 frame_start_time, frame_end_time, frame_elapsed_time;

// Add the global variables
bool quit = false;
pthread_mutex_t mutex;
SDL_Surface* trackSurface;
int clientSockets[MAX_PLAYERS];

typedef struct {
    int id;
    int x;
    int y;
} PlayerData;

PlayerData playersData[MAX_PLAYERS];

// player_thread_function będzie obsługiwać każdego gracza jako oddzielny wątek
void* player_thread_function(void* player_ptr) {
    Player* player = (Player*)player_ptr;

    while (!quit) {
        frame_start_time = SDL_GetTicks();

        pthread_mutex_lock(&mutex);
        player_update_position(player, trackSurface);
        pthread_mutex_unlock(&mutex);

        // Zaktualizuj dane gracza zamiast wysyłać je
        playersData[player->color].id = player->color;
        playersData[player->color].x = player->x;
        playersData[player->color].y = player->y;
        
        frame_end_time = SDL_GetTicks();
        frame_elapsed_time = frame_end_time - frame_start_time;

        if (frame_elapsed_time < FRAME_DURATION) {
            SDL_Delay(FRAME_DURATION - frame_elapsed_time);
        }
    }

    return NULL;
}

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
    
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int connectedPlayers = 0;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (connectedPlayers < MAX_PLAYERS) {
        int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (newSocket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        } else {
            printf("Connected with IP: %s, port: %hu\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            clientSockets[connectedPlayers] = newSocket;
            
            // Wysyłamy klientowi jego unikalne ID
        if (send(newSocket, &connectedPlayers, sizeof(int), 0) < 0) {
            perror("send");
            break;
        }
        
            connectedPlayers++;
        }
    }

    // Inicjalizacja SDL
    //SDL_Init(SDL_INIT_VIDEO);

    // Dodajemy wątki dla każdego gracza
    pthread_t player_threads[MAX_PLAYERS];
    pthread_mutex_init(&mutex, NULL);

    Player* players[MAX_PLAYERS];
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = player_create(START_X_POS + i*70, START_Y_POS);
        players[i]->color = i;

        // Inicjalizacja wątków dla każdego gracza
        pthread_create(&player_threads[i], NULL, player_thread_function, (void*)players[i]);
    }

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
        
        // Po zaktualizowaniu wszystkich graczy, wysyłamy informacje o nich do wszystkich klientów
        for (int i = 0; i < MAX_PLAYERS; i++) {
            int sent = send(clientSockets[i], &playersData, sizeof(playersData), 0);
            if (sent <= 0) {
                perror("send");
                break;
            }
        }
        
        for (int i = 0; i < MAX_PLAYERS; i++) {
          DrawPlayer(renderer,players[i]->x, players[i]->y,players[i]->color);
        }

        //DrawPlayer(renderer, player->x, player->y, player->color);
        //DrawPlayer(renderer, player1->x, player1->y, player1->color);
        //DrawPlayer(renderer, player2->x, player2->y, player2->color);
        //DrawPlayer(renderer, player3->x, player3->y);
        
        pthread_mutex_lock(&mutex);
        int current_corridor_lock_status = is_corridor_locked;
        int current_pitstop_lock_status = is_pitstop_locked;
        pthread_mutex_unlock(&mutex);
        
        for (int i = 0; i < NUM_TRACK_POINTS; i++) {
          DrawPoint(renderer, track_points[i]);
        }
        
        DrawLockStatusCircle(renderer, current_corridor_lock_status, -33);
        DrawLockStatusCircle(renderer, current_pitstop_lock_status, 20);

        SDL_RenderPresent(renderer);
        
        SDL_Delay(3);
        
        usleep(100000);
    }

    // Oczekiwanie na zakończenie wątków
    for (int i = 0; i < MAX_PLAYERS; i++) {
        pthread_join(player_threads[i], NULL);
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        close(clientSockets[i]);
    }

    close(serverSocket);
    SDL_Quit();

    return 0;
}

