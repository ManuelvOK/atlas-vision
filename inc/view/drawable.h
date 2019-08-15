#pragma once
#include <SDL2/SDL.h>

#include <view/positionable.h>

class Viewmodel;
class Model;
class Drawable : public Positionable {
protected:
    const Viewmodel *viewmodel;
public:
    Drawable(const Viewmodel *viewmodel) : viewmodel(viewmodel) {}
    virtual void update(const Model *model) { (void) model; }
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const = 0;
    virtual bool is_visible(int timestamp) const = 0;
    virtual ~Drawable() = default;
};
