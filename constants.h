#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL.h>

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

typedef struct {
    int x;
    int y;
} Point;

#define PLAYER_SQUARE_SIZE 20
#define PLAYER_SPEED 5
#define PLAYER_SPEED_LOADING_NITRO 1
#define PLAYER_SPEED_NITRO 8

#define NITRO_LAST_TIME 4

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 700

#define TRACK_MARGIN 130
#define INNER_MARGIN 250

#define START_X_POS 130
#define START_Y_POS 170

#define CHECKING_MARGIN 3

#define NUM_TRACK_POINTS 9

const Color CORRIDOR_IN_COLOR = { 0, 0, 255 };    // red
const Color CORRIDOR_OUT_COLOR = { 0, 255, 0 };  // green

const Color PITSTOP_IN_COLOR = { 0, 255, 255 };   // yellow
const Color PITSTOP_OUT_COLOR = { 255, 200, 0 }; // turkusowy

const Color DONT_ENTER_COLOR = { 255, 255, 255 }; // white

#endif /* CONSTANTS_H */
