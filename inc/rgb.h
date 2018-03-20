#pragma once
#include <SDL2/SDL.h>

class RGB {
public:
    int r = 0;
    int g = 0;
    int b = 0;

    RGB() = default;
    RGB(int grey) : r(grey), g(grey), b(grey) {}
    RGB(int r, int g, int b) : r(r), g(g), b(b) {}
    void activate(SDL_Renderer *renderer, int alpha = 255) const;
};
