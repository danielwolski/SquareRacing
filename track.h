#ifndef TRACK_H
#define TRACK_H

#include <SDL.h>
#include "graphics.h"
#include "constants.h"

extern SDL_Point track_points[NUM_TRACK_POINTS];

void track_init();
Uint32 GetPixel(SDL_Surface* surface, int x, int y);
bool CanMove(SDL_Surface* surface, int x, int y, int direction);

#endif // TRACK_H
