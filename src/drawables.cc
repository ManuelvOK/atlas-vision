#include <drawables.h>

#include <iostream>
#include <algorithm>

#include <SDL2/SDL2_gfxPrimitives.h>

Arrow::Arrow(std::array<short,9> arrow_coords_x, std::array<short,9> arrow_coords_y, int x, int y) :
    arrow_coords_x(arrow_coords_x), arrow_coords_y(arrow_coords_y), x(x), y(y) {
}

void Arrow::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    //constexpr int size = decltype(this->x).size();
    short pos_x[std::tuple_size<decltype(this->arrow_coords_x)>::value];
    short pos_y[std::tuple_size<decltype(this->arrow_coords_x)>::value];
    for (unsigned j = 0; j < this->arrow_coords_x.size(); ++j) {
        /* TODO: get rid of magic 5 */
        pos_x[j] = this->arrow_coords_x[j] / 5.0 + this->x + offset_x;
        pos_y[j] = this->arrow_coords_y[j] / 5.0 + this->y + offset_y;
    }
    filledPolygonRGBA(renderer, pos_x, pos_y, this->arrow_coords_x.size(), this->color.r, this->color.g,
                      this->color.b, 255);
}

void Rect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    SDL_Rect r = this->rect;
    r.x += offset_x;
    r.y += offset_y;

    this->color.activate(renderer);
    SDL_RenderFillRect(renderer, &r);

    /* draw border */
    if (not this->border) {
        return;
    }
    this->border_color.activate(renderer);
    SDL_RenderDrawRect(renderer, &r);
    /* since the drawn rect does not include the lower right point,
        * we have to draw it manually */
    SDL_RenderDrawPoint(renderer, r.x + r.w - 1, r.y + r.h - 1);
}

void Line::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    this->color.activate(renderer);
    SDL_RenderDrawLine(renderer, this->begin_x + offset_x, this->begin_y + offset_y,
                       this->end_x + offset_x, this->end_y + offset_y);
}
