#include <model/rgb.h>

void RGB::activate(SDL_Renderer *renderer, int alpha) const {
    SDL_SetRenderDrawColor(renderer, this->r, this->g, this->b, alpha);
}
