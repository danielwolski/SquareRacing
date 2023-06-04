#include "graphics.h"
#include "constants.h"

SDL_Texture* loadTexture(const char* filename, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
    return texture;
}

void DrawPlayer(SDL_Renderer* renderer, int x, int y, int color) {
    switch(color){
        case 0:
            SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
            break;
        case 1:
            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
            break;
        case 2:
            SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
            break;
        default:
            SDL_SetRenderDrawColor(renderer, 100, 100, 0, 255);
            break;
    }
    //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect player = { x, y, PLAYER_SQUARE_SIZE, PLAYER_SQUARE_SIZE };
    SDL_RenderFillRect(renderer, &player);
}

void DrawLockStatusCircle(SDL_Renderer* renderer, int lock_status, int offsetY) {
    int circleRadius = 16;
    int circleX = WINDOW_WIDTH - 70;
    int circleY = 50 + offsetY;

    // Wybierz kolor na podstawie wartoci lock_status
    if (lock_status == 0) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }

    // Rysuj okr¹g
    for (int w = 0; w < circleRadius * 2; w++) {
        for (int h = 0; h < circleRadius * 2; h++) {
            int dx = circleRadius - w;
            int dy = circleRadius - h;
            if ((dx * dx + dy * dy) <= (circleRadius * circleRadius)) {
                SDL_RenderDrawPoint(renderer, circleX + dx, circleY + dy);
            }
        }
    }
}

void DrawPoint(SDL_Renderer* renderer, SDL_Point p) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderDrawPoint(renderer, p.x, p.y);
    SDL_RenderDrawPoint(renderer, p.x - 1, p.y);
    SDL_RenderDrawPoint(renderer, p.x, p.y - 1);
    SDL_RenderDrawPoint(renderer, p.x - 1, p.y - 1);
}

bool IsColor(Uint32 pixel, SDL_PixelFormat* format, Color color) {
    Uint8 r, g, b;
    SDL_GetRGB(pixel, format, &r, &g, &b);
    return r == color.r && g == color.g && b == color.b;
}
