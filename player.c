#include <time.h>
#include "player.h"
#include "track.h"
#include "constants.h"

extern int is_corridor_locked;
extern int is_pitstop_locked;

Player* player_create(int x, int y) {
    Player* player = (Player*)malloc(sizeof(Player));
    player->x = x;
    player->y = y;
    player->speed = PLAYER_SPEED;
    player->current_track_point = 0;
    player->speed_increase_start = 0;
    player_update_direction(player);
    return player;
}

void player_update_direction(Player* player) {
    if (player->current_track_point + 1 >= sizeof(track_points) / sizeof(track_points[0])) {
        player->current_track_point = 0;
    }

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

    player->current_track_point++;
}


void player_update_position(Player* player, SDL_Surface* trackSurface) {
    int newX = player->x, newY = player->y;
    int speed = player->speed;

    if (player->directionY == -1) {
        newY -= speed;
    }
    if (player->directionY == 1) {
        newY += speed;
    }

    if (CanMove(trackSurface, newX, newY, 1) && CanMove(trackSurface, newX, newY, 8) &&
        CanMove(trackSurface, newX, newY, 5) && CanMove(trackSurface, newX, newY, 4)) {
        player->y = newY;
    }

    newX = player->x;

    if (player->directionX == -1) {
        newX -= speed;
    }
    if (player->directionX == 1) {
        newX += speed;
    }

    if (CanMove(trackSurface, newX, player->y, 7) && CanMove(trackSurface, newX, player->y, 6) &&
        CanMove(trackSurface, newX, player->y, 3) && CanMove(trackSurface, newX, player->y, 2)) {
        player->x = newX;
    }

    // Check whether the speed increase has ended
    if (player->speed_increase_start != 0 && time(NULL) - player->speed_increase_start >= NITRO_LAST_TIME) {
        player->speed = PLAYER_SPEED;
        player->speed_increase_start = 0;
    }

    // Check whether player un/locks pitstop
    Uint32 colorTop = GetPixel(trackSurface, player->x, newY - 1);
    if (IsColor(colorTop, trackSurface->format, PITSTOP_IN_COLOR)) {
        is_pitstop_locked = 1;
        player->speed = PLAYER_SPEED_LOADING_NITRO;
    }
    else if (IsColor(colorTop, trackSurface->format, PITSTOP_OUT_COLOR)) {
        is_pitstop_locked = 0;
        player->speed = PLAYER_SPEED_NITRO;
        player->speed_increase_start = time(NULL);
    }

    // Check whether player un/locks the corridor
    Uint32 colorLeft = GetPixel(trackSurface, newX - 1, player->y);
    if (IsColor(colorLeft, trackSurface->format, CORRIDOR_IN_COLOR)) {
        is_corridor_locked = 1;
    }
    else if (IsColor(colorLeft, trackSurface->format, CORRIDOR_OUT_COLOR)) {
        is_corridor_locked = 0;
    }

    player_update_direction(player);
}

void player_destroy(Player* player) {
    if (player) {
        free(player);
    }
}
