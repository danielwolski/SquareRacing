#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

#define PLAYER_SQUARE_SIZE 20
#define PLAYER_SPEED 5

#define WINDOW_WIDTH 1800
#define WINDOW_HEIGHT 700

#define TRACK_MARGIN 100
#define INNER_MARGIN 250

#define TIMER_ID 1
#define TIMER_INTERVAL 3

void DrawTrack(HDC hdc) {
    //czarny prostokat trasy
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH oldBrush = SelectObject(hdc, brush);
    Rectangle(hdc, TRACK_MARGIN, TRACK_MARGIN, WINDOW_WIDTH - TRACK_MARGIN, WINDOW_HEIGHT - TRACK_MARGIN);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);

    //bialy prostokat wewnatrz trasy
    brush = CreateSolidBrush(RGB(255, 255, 255));
    oldBrush = SelectObject(hdc, brush);
    Rectangle(hdc, INNER_MARGIN, INNER_MARGIN, WINDOW_WIDTH - INNER_MARGIN, WINDOW_HEIGHT - INNER_MARGIN);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

void DrawPlayer(HDC hdc, int x, int y) {
    //wspolrzedne x oraz y wyznaczaja gorny lewy rog kwadratu gracza!
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 255));
    HBRUSH oldBrush = SelectObject(hdc, brush);
    Rectangle(hdc, x, y, x + PLAYER_SQUARE_SIZE, y + PLAYER_SQUARE_SIZE);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

bool IsWhiteColor(COLORREF color) {
    return GetRValue(color) == 255 && GetGValue(color) == 255 && GetBValue(color) == 255;
}

//funkcja sprawdzajaca odpowiedni piksel w zale¿noœci od parametru direction
//     1 
//   #####
// 2 ##### 4
//   #####
//     3
bool CanMove(HWND hwnd, int x, int y, int direction) {
    HDC hdcWindow = GetDC(hwnd);
    COLORREF color = RGB(255, 255, 255); //domyslnie bialy
    switch (direction) {
    case 1: //sprawdzenie piksela nad gorna sciana gracza
        color = GetPixel(hdcWindow, x + PLAYER_SQUARE_SIZE / 2, y - 1);
        break;
    case 2: //sprawdzenie piksela przed lewa sciana gracza
        color = GetPixel(hdcWindow, x - 1, y + PLAYER_SQUARE_SIZE / 2);
        break;
    case 3: //sprawdzenie piksela pod dolna sciana gracza
        color = GetPixel(hdcWindow, x + PLAYER_SQUARE_SIZE / 2, y + PLAYER_SQUARE_SIZE + 1);
        break;
    case 4: //sprawdzenie piksela za prawa sciana gracza
        color = GetPixel(hdcWindow, x + PLAYER_SQUARE_SIZE + 1, y + PLAYER_SQUARE_SIZE / 2);
        break;
    default:
        break;
    }
    ReleaseDC(hwnd, hdcWindow);

    return !IsWhiteColor(color); //gracz nie mo¿e wjechaæ na bia³e pole
}

void UpdateSquarePosition(HWND hwnd, int* x, int* y, const bool keys[]) {
    static int speed = PLAYER_SPEED;
    int newX, newY;//nowe wartoœci wspó³rzêdnych po przesuniêciu o wartoœæ speed

    if (keys['W']) {
        newY = *y - speed;
        if (CanMove(hwnd, *x, newY, 1))
            *y = newY;
    }
    if (keys['S']) {
        newY = *y + speed;
        if (CanMove(hwnd, *x, newY, 3))
            *y = newY;
    }
    if (keys['A']) {
        newX = *x - speed;
        if (CanMove(hwnd, newX, *y, 2))
            *x = newX;
    }
    if (keys['D']) {
        newX = *x + speed;
        if (CanMove(hwnd, newX, *y, 4))
            *x = newX;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //pozycja startowa gracza
    static int x = INNER_MARGIN - 10;
    static int y = INNER_MARGIN - 80;

    static bool keys[256] = { 0 };

    switch (uMsg) {
    case WM_KEYDOWN:
        keys[wParam] = true;
        break;

    case WM_KEYUP:
        keys[wParam] = false;
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            UpdateSquarePosition(hwnd, &x, &y, keys);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawTrack(hdc);
        DrawPlayer(hdc, x, y);
        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main() {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MyWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(L"MyWindowClass", L"Racing Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
