#ifndef TRACK_H
#define TRACK_H

#include <SDL.h>
#include "graphics.h"
#include "constants.h"

extern SDL_Point track_points[NUM_TRACK_POINTS];
extern int is_corridor_locked;
extern int is_pitstop_locked;

Uint32 GetPixel(SDL_Surface* surface, int x, int y);
bool CanMove(SDL_Surface* surface, int x, int y, int direction, int did_player_lock_pitstop, int did_player_lock_corridor);

#endif // TRACK_H
