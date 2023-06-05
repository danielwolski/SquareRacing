#include <time.h>
#include <pthread.h>
#include "player.h"
#include "track.h"
#include "constants.h"

pthread_mutex_t lock; 

Player* player_create(int x, int y) {
    Player* player = (Player*)malloc(sizeof(Player));
    player->x = x;
    player->y = y;
    player->speed = PLAYER_SPEED;
    player->current_track_point = 0;
    player->speed_increase_start = 0;
    player->did_player_lock_corridor = 0;
    player->did_player_lock_pitstop = 0;
    return player;
}

void player_update_direction(Player* player) {
    if (player->current_track_point + 1 >= sizeof(track_points) / sizeof(track_points[0])) {
        player->current_track_point = 0;
    }
    
    if (player->current_track_point == 7 && is_pitstop_locked == 1)
        player->current_track_point = 0;

    Point next;
    next.x = track_points[player->current_track_point].x;
    next.y = track_points[player->current_track_point].y;
    int diffX = next.x - player->x;
    int diffY = next.y - player->y;

    if (abs(diffX) <= CHECKING_MARGIN) {
        player->directionX = 0;
    }
    else {
        player->directionX = diffX > 0 ? 1 : -1;
    }

    if (abs(diffY) <= CHECKING_MARGIN) {
        player->directionY = 0;
    }
    else {
        player->directionY = diffY > 0 ? 1 : -1;
    }
}


void player_update_position(Player* player, SDL_Surface* trackSurface) {
    pthread_mutex_init(&lock, NULL);
    static int speed = PLAYER_SPEED;
    int newX = player->x, newY = player->y;

    // Check if the player reached the current track point
    Point current_point;
    current_point.x = track_points[player->current_track_point].x;
    current_point.y = track_points[player->current_track_point].y;

    //printf("x_player: %d y_player: %d\n", player->x, player->y);
    //printf("x: %d y: %d\n", current_point.x, current_point.y);

    if (abs(player->x - current_point.x) <= CHECKING_MARGIN && abs(player->y - current_point.y) <= CHECKING_MARGIN) {
        //printf("SUCCESS\n");
        player->current_track_point++;
    }

    player_update_direction(player);

    // Check whether the speed increase has ended
    if (player->speed_increase_start != 0 && time(NULL) - player->speed_increase_start >= NITRO_LAST_TIME) {
        player->speed = PLAYER_SPEED;
        player->speed_increase_start = 0;
    }

    if (player->directionY == -1) {
        newY -= player->speed;
    }
    if (player->directionY == 1) {
        newY += player->speed;
    }

    // Check whether player un/locks pitstop
    if (player->directionY == -1) {
        if (trackSurface == NULL)
          printf("NO SURFACE HERE\n");
        Uint32 colorTop = GetPixel(trackSurface, player->x, newY - 1);
        //printf("Raw colorTop: %u\n", colorTop);
        Uint8 r, g, b;
        SDL_GetRGB(colorTop, trackSurface->format, &r, &g, &b);
        //printf("RGB: (%d, %d, %d)\n", r, g, b);

        if (IsColor(colorTop, trackSurface->format, PITSTOP_IN_COLOR) && is_pitstop_locked == 0) {
            is_pitstop_locked = 1;
            player->did_player_lock_pitstop = 1;
            player->speed = PLAYER_SPEED_LOADING_NITRO;
        }
        else if (IsColor(colorTop, trackSurface->format, PITSTOP_OUT_COLOR)) {
            is_pitstop_locked = 0;
            player->did_player_lock_pitstop = 0;
            player->speed = PLAYER_SPEED_NITRO;
            player->speed_increase_start = time(NULL);
        }
    }

    if (CanMove(trackSurface, newX, newY, 1, player->did_player_lock_pitstop, player->did_player_lock_corridor) && CanMove(trackSurface, newX, newY, 8, player->did_player_lock_pitstop, player->did_player_lock_corridor) &&
        CanMove(trackSurface, newX, newY, 5, player->did_player_lock_pitstop, player->did_player_lock_corridor) && CanMove(trackSurface, newX, newY, 4, player->did_player_lock_pitstop, player->did_player_lock_corridor)) {
        player->y = newY;
    }

    newX = player->x;

    if (player->directionX == -1) {
        newX -= player->speed;
    }
    if (player->directionX == 1) {
        newX += player->speed;
    }
    
    // Check whether player un/locks the corridor
    if (player->directionX == -1) {
        Uint32 colorTop = GetPixel(trackSurface, newX - 1, player->y);
        //printf("Raw colorLeft: %u\n", colorTop);
        Uint8 r, g, b;
        SDL_GetRGB(colorTop, trackSurface->format, &r, &g, &b);
        //printf("RGB: (%d, %d, %d)\n", r, g, b);

        pthread_mutex_lock(&lock);
        if (IsColor(colorTop, trackSurface->format, CORRIDOR_IN_COLOR) && is_corridor_locked == 0){
            //pthread_mutex_lock(&lock);
            is_corridor_locked = 1;
            player->did_player_lock_corridor = 1;
            //pthread_mutex_unlock(&lock);
            }
        else if (IsColor(colorTop, trackSurface->format, CORRIDOR_OUT_COLOR)){
            is_corridor_locked = 0;
            player->did_player_lock_corridor = 0;
            }
        pthread_mutex_unlock(&lock);
    }

    if (CanMove(trackSurface, newX, player->y, 7, player->did_player_lock_pitstop, player->did_player_lock_corridor) && CanMove(trackSurface, newX, player->y, 6, player->did_player_lock_pitstop, player->did_player_lock_corridor) &&
        CanMove(trackSurface, newX, player->y, 3, player->did_player_lock_pitstop, player->did_player_lock_corridor) && CanMove(trackSurface, newX, player->y, 2, player->did_player_lock_pitstop, player->did_player_lock_corridor)) {
        player->x = newX;
}

    
}


void player_destroy(Player* player) {
    if (player) {
        free(player);
    }
}
