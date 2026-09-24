/* SPDX-License-Identifier: BSD-3-Clause */
#include "bondwell.h"
static BYTE pixels[640 * 256];
#ifdef _WIN32
#include <windows.h>
static HWND window;
static DWORD rgb[640 * 256];
static bool closing;
static int special(unsigned key)
{
    if (key >= VK_F1 && key <= VK_F6) return 34 + key - VK_F1;
    if (key >= VK_F7 && key <= VK_F10) return 30 + key - VK_F7;
    if (key >= VK_F11 && key <= VK_F12) return 26 + key - VK_F11;
    switch (key) {
    case VK_LEFT: return 40; case VK_RIGHT: return 41;
    case VK_UP: return 48; case VK_DOWN: return 49;
    case VK_DELETE: return 54; case VK_CAPITAL: return 25;
    default: return -1;
    }
}
static LRESULT CALLBACK procedure(HWND h, UINT msg, WPARAM w, LPARAM l)
{
    switch (msg) {
    case WM_CLOSE: closing = true; return 0;
    case WM_KEYDOWN: {
        int matrix = special((unsigned)w);
        if (matrix >= 0) bw_queue_key(matrix, (GetKeyState(VK_SHIFT) & 0x8000) != 0,
                               (GetKeyState(VK_CONTROL) & 0x8000) != 0);
        return 0;
    }
    case WM_CHAR: bw_key_ascii((unsigned)w); return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(h, &paint);
        RECT rect; GetClientRect(h, &rect);
        unsigned height; bw_render(pixels, &height);
        for (unsigned i = 0; i < height * 640; ++i)
            rgb[i] = pixels[i] ? 0x00ffb840 : 0x000c0903;
        BITMAPINFO info = {0};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = 640; info.bmiHeader.biHeight = -(LONG)height;
        info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        SetStretchBltMode(dc, COLORONCOLOR);
        StretchDIBits(dc, 0, 0, rect.right, rect.bottom, 0, 0, 640, height,
                      rgb, &info, DIB_RGB_COLORS, SRCCOPY);
        EndPaint(h, &paint); return 0;
    }
    }
    return DefWindowProc(h, msg, w, l);
}
bool bw_ui_open(void)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = procedure; wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "Z80PackBondwell"; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClass(&wc)) return false;
    window = CreateWindow(wc.lpszClassName, "Bondwell 12/14 - z80pack",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 420, NULL, NULL, wc.hInstance, NULL);
    return window != NULL;
}
bool bw_ui_poll(void)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg); DispatchMessage(&msg);
    }
    return !closing;
}
void bw_ui_draw(void) { InvalidateRect(window, NULL, FALSE); UpdateWindow(window); }
void bw_ui_close(void) { if (window) DestroyWindow(window); }
#elif defined(BW_SDL)
#include <SDL.h>
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static uint32_t rgb[640 * 256];
bool bw_ui_open(void)
{
    if (SDL_Init(SDL_INIT_VIDEO)) return false;
    window = SDL_CreateWindow("Bondwell 12/14 - z80pack", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 960, 360, SDL_WINDOW_RESIZABLE);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) return false;
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING, 640, 256);
    SDL_StartTextInput();
    return texture != NULL;
}
bool bw_ui_poll(void)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return false;
        if (e.type == SDL_TEXTINPUT)
            for (const unsigned char *p = (const unsigned char *)e.text.text; *p; ++p) bw_key_ascii(*p);
        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode k = e.key.keysym.sym;
            bool ctrl = (e.key.keysym.mod & KMOD_CTRL) != 0;
            int matrix = -1;
            if (k >= SDLK_F1 && k <= SDLK_F6) matrix = 34 + k - SDLK_F1;
            if (k >= SDLK_F7 && k <= SDLK_F10) matrix = 30 + k - SDLK_F7;
            if (k >= SDLK_F11 && k <= SDLK_F12) matrix = 26 + k - SDLK_F11;
            if (k == SDLK_LEFT) matrix = 40;
            if (k == SDLK_RIGHT) matrix = 41;
            if (k == SDLK_UP) matrix = 48;
            if (k == SDLK_DOWN) matrix = 49;
            if (k == SDLK_CAPSLOCK) matrix = 25;
            if (matrix >= 0) bw_queue_key(matrix, (e.key.keysym.mod & KMOD_SHIFT) != 0, ctrl);
            else if (ctrl && k >= 'a' && k <= 'z') bw_key_ascii(k - 'a' + 1);
            else if (k == SDLK_RETURN || k == SDLK_BACKSPACE || k == SDLK_TAB || k == SDLK_ESCAPE || k == SDLK_DELETE)
                bw_key_ascii((unsigned)k);
        }
    }
    return true;
}
void bw_ui_draw(void)
{
    unsigned height; bw_render(pixels, &height);
    for (unsigned i = 0; i < 640 * 256; ++i) rgb[i] = pixels[i] ? 0xffffb840 : 0xff0c0903;
    SDL_UpdateTexture(texture, NULL, rgb, 640 * sizeof *rgb);
    SDL_Rect src = {0,0,640,(int)height};
    SDL_RenderClear(renderer); SDL_RenderCopy(renderer, texture, &src, NULL); SDL_RenderPresent(renderer);
}
void bw_ui_close(void)
{
    SDL_DestroyTexture(texture); SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window); SDL_Quit();
}
#else
bool bw_ui_open(void) { (void)pixels; return false; }
bool bw_ui_poll(void) { return true; }
void bw_ui_draw(void) {}
void bw_ui_close(void) {}
#endif
