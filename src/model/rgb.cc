#include <model/rgb.h>

void RGB::activate(SDL_Renderer *renderer, int alpha) const {
    SDL_SetRenderDrawColor(renderer, this->_r, this->_g, this->_b, alpha);
}
