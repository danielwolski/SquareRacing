#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include "player.h"

SDL_Texture* loadTexture(const char* filename, SDL_Renderer* renderer);
void DrawPlayer(SDL_Renderer* renderer, int x, int y);
void DrawLockStatusCircle(SDL_Renderer* renderer, int lock_status, int offsetY);
bool IsColor(Uint32 pixel, SDL_PixelFormat* format, Color color);

#endif // GRAPHICS_H