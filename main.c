#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define PLAYER_SQUARE_SIZE 20
#define PLAYER_SPEED 5

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 700

#define TRACK_MARGIN 130
#define INNER_MARGIN 250

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

const Color CORRIDOR_IN_COLOR = {0, 0, 255};    // tutaj powinno byc 255,0,0 ale program traktuje czerwony jako 0,0,255(????) do naprawienia - robi tak tylko jak sprawdza wartosc piksela, maluje na czerwono juz normalnie
const Color CORRIDOR_OUT_COLOR = {0, 255, 0};  // green

const Color PITSTOP_IN_COLOR = {255, 255, 0};   // yellow
const Color PITSTOP_OUT_COLOR = {0, 255, 255}; // turkusowy

const Color DONT_ENTER_COLOR = {255, 255, 255}; // white

int IS_CORRIDOR_LOCKED = 0;

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


void DrawCorridorLockStatusCircle(SDL_Renderer *renderer) {
    int circleRadius = 20;
    int circleX = WINDOW_WIDTH - 50;
    int circleY = 50;

    // Wybierz kolor na podstawie wartoœci IS_CORRIDOR_LOCKED
    if (IS_CORRIDOR_LOCKED == 0) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else {
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

bool IsColor(Uint32 pixel, SDL_PixelFormat *format, Color color) {
    Uint8 r, g, b;
    SDL_GetRGB(pixel, format, &r, &g, &b);
    return r == color.r && g == color.g && b == color.b;
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

void UpdateSquarePosition(int *x, int *y, const bool keys[], SDL_Surface *trackSurface) {
    static int speed = PLAYER_SPEED;
    int newX = *x, newY = *y;

    if (keys[SDL_SCANCODE_W]) {
        newY -= speed;
    }
    if (keys[SDL_SCANCODE_S]) {
        newY += speed;
    }

    if (CanMove(trackSurface, newX, newY, 1) && CanMove(trackSurface, newX, newY, 8) &&
        CanMove(trackSurface, newX, newY, 5) && CanMove(trackSurface, newX, newY, 4)) {
        *y = newY;
    }

    newX = *x;

    if (keys[SDL_SCANCODE_A]) {
        newX -= speed;
    }
    if (keys[SDL_SCANCODE_D]) {
        newX += speed;
    }
    
    //testowanie - jaki kolor jest na mapie?
    Uint32 testColor = GetPixel(trackSurface, newX - 1, *y);
    Uint8 r, g, b;
    SDL_GetRGB(testColor, trackSurface->format, &r, &g, &b);
    printf("Color at newX-1, y: (%u, %u, %u)\n", r, g, b);

    testColor = GetPixel(trackSurface, newX - 1, *y + PLAYER_SQUARE_SIZE - 1);
    SDL_GetRGB(testColor, trackSurface->format, &r, &g, &b);
    printf("Color at newX-1, y+player_square_size-1: (%u, %u, %u)\n", r, g, b);


    // SprawdŸ, czy gracz napotka³ czerwony kolor podczas jazdy w lewo
    if (keys[SDL_SCANCODE_A]) {
        Uint32 colorTop = GetPixel(trackSurface, newX - 1, *y);
        printf("Raw colorTop: %u\n", colorTop);
        Uint8 r, g, b;
        SDL_GetRGB(colorTop, trackSurface->format, &r, &g, &b);
        printf("RGB: (%d, %d, %d)\n", r, g, b);
        if (IsColor(colorTop, trackSurface->format, CORRIDOR_IN_COLOR)) {
            IS_CORRIDOR_LOCKED = 1;
        }
    }

    
    if (CanMove(trackSurface, newX, *y, 7) && CanMove(trackSurface, newX, *y, 6) &&
        CanMove(trackSurface, newX, *y, 3) && CanMove(trackSurface, newX, *y, 2)) {
        *x = newX;
    }
    
   
}


int main(int argc, char *argv[]) {

    SDL_Surface *trackSurface = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Racing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* trackTexture = loadTexture("track.png", renderer);
    if (!trackTexture) {
        // Handle error
    }

    int x = INNER_MARGIN - 10;
    int y = INNER_MARGIN - 80;

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

        UpdateSquarePosition(&x, &y, keys, trackSurface);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, trackTexture, NULL, NULL);  // Assumes track texture size matches the window size

        DrawPlayer(renderer, x, y);
        
        DrawCorridorLockStatusCircle(renderer);

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