#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <time.h>

#define PLAYER_SQUARE_SIZE 20
#define PLAYER_SPEED 3
#define PLAYER_SPEED_LOADING_NITRO 1
#define PLAYER_SPEED_NITRO 6

#define NITRO_LAST_TIME 4

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 700

#define TRACK_MARGIN 130
#define INNER_MARGIN 250

#define NUM_TRACK_POINTS 10

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

typedef struct {
    int x;
    int y;
} Point;

Point track_points[10];
int directionX = 0, directionY = 0;
int current_track_point = 0;


//kolory sa odwrocone zamiast RGB jest BGR
const Color CORRIDOR_IN_COLOR = {0, 0, 255};    // red
const Color CORRIDOR_OUT_COLOR = {0, 255, 0};  // green

const Color PITSTOP_IN_COLOR = {0, 255, 255};   // yellow
const Color PITSTOP_OUT_COLOR = {255, 200, 0}; // turkusowy

const Color DONT_ENTER_COLOR = {255, 255, 255}; // white

int IS_CORRIDOR_LOCKED = 0;
int IS_PITSTOP_LOCKED = 0;

time_t speed_increase_start = 0;

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

void DrawPlayer(SDL_Renderer *renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect player = {x, y, PLAYER_SQUARE_SIZE, PLAYER_SQUARE_SIZE};
    SDL_RenderFillRect(renderer, &player);
}


void DrawLockStatusCircle(SDL_Renderer *renderer, int lock_status, int offsetY) {
    int circleRadius = 20;
    int circleX = WINDOW_WIDTH - 70;
    int circleY = 50 + offsetY;

    // Wybierz kolor na podstawie wartości lock_status
    if (lock_status == 0) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }

    // Rysuj okrąg
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

bool IsColor(Uint32 pixel, SDL_PixelFormat *format, Color color) {
    Uint8 r, g, b;
    SDL_GetRGB(pixel, format, &r, &g, &b);
    return r == color.r && g == color.g && b == color.b;
}

void DrawPoint(SDL_Renderer* renderer, SDL_Point p) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderDrawPoint(renderer, p.x, p.y);
    SDL_RenderDrawPoint(renderer, p.x - 1, p.y);
    SDL_RenderDrawPoint(renderer, p.x, p.y - 1);
    SDL_RenderDrawPoint(renderer, p.x - 1, p.y - 1);
}

Uint32 GetPixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16 *)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
        case 4:
            return *(Uint32 *)p;
        default:
            return 0;
    }
}

void UpdateDirection(int x, int y) {
    if (current_track_point + 1 >= sizeof(track_points) / sizeof(track_points[0])) {
        return;
    }

    Point next = track_points[current_track_point + 1];

    directionX = next.x > x ? 1 : (next.x < x ? -1 : 0);
    directionY = next.y > y ? 1 : (next.y < y ? -1 : 0);
}



bool CanMove(SDL_Surface *surface, int x, int y, int direction) {
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

void UpdateSquarePosition(int *x, int *y, const bool keys[], SDL_Surface *trackSurface, int directionX, int directionY) {
    static int speed = PLAYER_SPEED;
    int newX = *x, newY = *y;
    
    // Sprawdź, czy gracz dotarł do następnego punktu ścieżki
    Point next_point = track_points[current_track_point + 1];
    if (abs(*x - next_point.x) <= PLAYER_SPEED && abs(*y - next_point.y) <= PLAYER_SPEED) {
        current_track_point++;
        UpdateDirection(*x,*y);
    }

    // Check whether the speed increase has ended
    if (speed_increase_start != 0 && time(NULL) - speed_increase_start >= NITRO_LAST_TIME) {
        speed = PLAYER_SPEED;
        speed_increase_start = 0;
    }

    if (directionY == -1) {
        newY -= speed;
    }
    if (directionY == 1) {
        newY += speed;
    }
    
    // Check whether player un/locks pitstop
    if (directionY == -1) {
        Uint32 colorTop = GetPixel(trackSurface, *x, newY-1);
        printf("Raw colorTop: %u\n", colorTop);
        Uint8 r, g, b;
        SDL_GetRGB(colorTop, trackSurface->format, &r, &g, &b);
        printf("RGB: (%d, %d, %d)\n", r, g, b);
        
        if (IsColor(colorTop, trackSurface->format, PITSTOP_IN_COLOR)){
            IS_PITSTOP_LOCKED = 1;
            speed = PLAYER_SPEED_LOADING_NITRO;
            }
        else if (IsColor(colorTop, trackSurface->format, PITSTOP_OUT_COLOR)){
            IS_PITSTOP_LOCKED = 0;
            speed = PLAYER_SPEED_NITRO;
            speed_increase_start = time(NULL);
            }
    }

    if (CanMove(trackSurface, newX, newY, 1) && CanMove(trackSurface, newX, newY, 8) &&
        CanMove(trackSurface, newX, newY, 5) && CanMove(trackSurface, newX, newY, 4)) {
        *y = newY;
    }

    newX = *x;

    if (directionX == -1) {
        newX -= speed;
    }
    if (directionX == 1) {
        newX += speed;
    }


    // Check whether player un/locks the corridor
    if (directionX == -1) {
        Uint32 colorTop = GetPixel(trackSurface, newX - 1, *y);
        printf("Raw colorLeft: %u\n", colorTop);
        Uint8 r, g, b;
        SDL_GetRGB(colorTop, trackSurface->format, &r, &g, &b);
        printf("RGB: (%d, %d, %d)\n", r, g, b);
        
        if (IsColor(colorTop, trackSurface->format, CORRIDOR_IN_COLOR))
            IS_CORRIDOR_LOCKED = 1;
        else if (IsColor(colorTop, trackSurface->format, CORRIDOR_OUT_COLOR))
            IS_CORRIDOR_LOCKED = 0;
    }

    
    if (CanMove(trackSurface, newX, *y, 7) && CanMove(trackSurface, newX, *y, 6) &&
        CanMove(trackSurface, newX, *y, 3) && CanMove(trackSurface, newX, *y, 2)) {
        *x = newX;
    }
    
   
}


int main(int argc, char *argv[]) {

    SDL_Point track_points[NUM_TRACK_POINTS] = {
        {150, 150},
        {300, 150},
        {1000, 150},
        {1200, 200},
        {1200, 500},
        {1000, 540},
        {510, 540},
        {75, 485},
        {75, 210},
        {135, 180}
    };
    

    SDL_Surface *trackSurface = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Racing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* trackTexture = loadTexture("track.png", renderer);
    if (!trackTexture) {
        // Handle error
    }

    int x = 150;
    int y = 150;

    bool keys[SDL_NUM_SCANCODES] = {false};
    bool running = true;
    
    trackSurface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    
    if (!trackSurface) {
    printf("Failed to create trackSurface: %s\n", SDL_GetError());
} else {
    printf("trackSurface created: %p\n", trackSurface);
}


    SDL_Texture* target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_SetRenderTarget(renderer, target);
    SDL_RenderCopy(renderer, trackTexture, NULL, NULL);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, trackSurface->pixels, trackSurface->pitch);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_DestroyTexture(target);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    keys[event.key.keysym.scancode] = true;
                    break;
                case SDL_KEYUP:
                    keys[event.key.keysym.scancode] = false;
                    break;
                default:
                    break;
            }
        }

        UpdateSquarePosition(&x, &y, keys, trackSurface, directionX, directionY);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, trackTexture, NULL, NULL);  // Assumes track texture size matches the window size

        DrawPlayer(renderer, x, y);
        
            for (int i = 0; i < NUM_TRACK_POINTS; i++) {
            DrawPoint(renderer, track_points[i]);
            }
        
        DrawLockStatusCircle(renderer, IS_CORRIDOR_LOCKED, -33);
        DrawLockStatusCircle(renderer, IS_PITSTOP_LOCKED, 20);


        SDL_RenderPresent(renderer);
        
        SDL_Delay(3);
    }
    
  SDL_FreeSurface(trackSurface);
  SDL_DestroyTexture(trackTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
