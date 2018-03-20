#pragma once
#include <SDL2/SDL.h>

class Drawable {
public:
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const = 0;
    virtual ~Drawable() = default;
};
