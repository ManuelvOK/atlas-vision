#pragma once
#include <SDL2/SDL.h>

class Viewmodel;
class Drawable {
protected:
    const Viewmodel *viewmodel;
public:
    Drawable(const Viewmodel *viewmodel) : viewmodel(viewmodel) {}
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const = 0;
    virtual bool is_visible(int timestamp) const = 0;
    virtual ~Drawable() = default;
};
