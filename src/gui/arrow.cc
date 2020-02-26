#include <gui/arrow.h>

#include <SDL2/SDL2_gfxPrimitives.h>

void Arrow::draw(SDL_Renderer *renderer, SDL_GUI::Position position) const {
    /* TODO: get rid of magic 9 */
    short pos_x[9];
    short pos_y[9];

    for (unsigned i = 0; i < 9; ++i) {
        /* TODO: get rid of magic 5 */
        pos_x[i] = this->_coords.at(this->_direction).first[i] / 5.0 + position._x;
        pos_y[i] = this->_coords.at(this->_direction).second[i] / 5.0 + position._y;
    }
    filledPolygonRGBA(renderer, pos_x, pos_y, 9, this->_current_style->_color._r,
                      this->_current_style->_color._g, this->_current_style->_color._b, 255);
}
