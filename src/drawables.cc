#include <drawables.h>

#include <SDL2/SDL2_gfxPrimitives.h>

void Arrow::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    short pos_x[this->x.size()];
    short pos_y[this->y.size()];
    SDL_Rect r = this->position;
    for (int j = 0; j < this->x.size(); ++j) {
        /* TODO: get rid of magic 5 */
        pos_x[j] = this->x[j] / 5.0 + r.x + offset_x;
        pos_y[j] = this->y[j] / 5.0 + r.y + offset_y;
    }
    filledPolygonRGBA(renderer, pos_x, pos_y, this->x.size(), this->color.r, this->color.g,
                      this->color.b, 255);
}

void Rect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    SDL_Rect r = this->rect;
    r.x += offset_x;
    r.y +=offset_y;

    this->color.activate(renderer);
    SDL_RenderFillRect(renderer, &r);

    /* draw border */
    if (this->border) {
        this->border_color.activate(renderer);
        SDL_RenderDrawRect(renderer, &r);
    }
}

void Line::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    this->color.activate(renderer);
    SDL_RenderDrawLine(renderer, this->begin_x + offset_x, this->begin_y + offset_y,
                       this->end_x + offset_x, this->end_y + offset_y);
}
