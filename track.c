#include "track.h"
#include "constants.h"

SDL_Point track_points[NUM_TRACK_POINTS] = {
        {181, 149},
        {300, 150},
        {1000, 150},
        {1200, 200},
        {1200, 500},
        {1000, 540},
        {510, 540},
        {75, 485},
        {75, 210},
};

Uint32 GetPixel(SDL_Surface* surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        return *p;
    case 2:
        return *(Uint16*)p;
    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            return p[0] << 16 | p[1] << 8 | p[2];
        }
        else {
            return p[0] | p[1] << 8 | p[2] << 16;
        }
    case 4:
        return *(Uint32*)p;
    default:
        return 0;
    }
}

bool CanMove(SDL_Surface* surface, int x, int y, int direction) {
    Uint32 color = 0;

    switch (direction) {
    case 1:
        color = GetPixel(surface, x + PLAYER_SQUARE_SIZE, y - 1);
        break;
    case 2:
        color = GetPixel(surface, x + PLAYER_SQUARE_SIZE + 1, y);
        break;
    case 3:
        color = GetPixel(surface, x + PLAYER_SQUARE_SIZE + 1, y + PLAYER_SQUARE_SIZE);
        break;
    case 4:
        color = GetPixel(surface, x + PLAYER_SQUARE_SIZE, y + PLAYER_SQUARE_SIZE);
        break;
    case 5:
        color = GetPixel(surface, x, y + PLAYER_SQUARE_SIZE + 1);
        break;
    case 6:
        color = GetPixel(surface, x - 1, y + PLAYER_SQUARE_SIZE);
        break;
    case 7:
        color = GetPixel(surface, x - 1, y);
        break;
    case 8:
        color = GetPixel(surface, x, y - 1);
        break;
    default:
        break;
    }

    return !IsColor(color, surface->format, DONT_ENTER_COLOR);
}
