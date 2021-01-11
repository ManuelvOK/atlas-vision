#include <gui/arrow.h>

#include <algorithm>

#include <SDL2/SDL2_gfxPrimitives.h>

Arrow::Arrow(SDL_GUI::Position position, Direction direction) :
    Drawable("Arrow", position),
    _direction(direction) {
    this->init();
}

Arrow::Arrow(std::string type, SDL_GUI::Position position, Direction direction) :
    Drawable(type, position),
    _direction(direction) {
    this->init();
}

void Arrow::init() {
    for (unsigned i = 0; i < 9; ++i) {
        /* TODO: get rid of magic 5 */
        this->_coords_x[i] = this->_coords.at(this->_direction).first[i] / 5.0;
        this->_coords_y[i] = this->_coords.at(this->_direction).second[i] / 5.0;
    }
    short min_x = *std::min_element(this->_coords_x.begin(), this->_coords_x.end());
    short max_x = *std::max_element(this->_coords_x.begin(), this->_coords_x.end());
    short min_y = *std::min_element(this->_coords_y.begin(), this->_coords_y.end());
    short max_y = *std::max_element(this->_coords_y.begin(), this->_coords_y.end());

    this->_width = max_x - min_x;
    this->_height = max_y - min_y;
}

void Arrow::draw(SDL_Renderer *renderer, SDL_GUI::Position position) const {
    /* TODO: get rid of magic 9 */
    short pos_x[9];
    short pos_y[9];

    for (unsigned i = 0; i < 9; ++i) {
        /* TODO: get rid of magic 5 */
        pos_x[i] = this->_coords_x[i] + position._x;
        pos_y[i] = this->_coords_y[i] + position._y;
    }
    filledPolygonRGBA(renderer, pos_x, pos_y, 9, this->_current_style->_color._r,
                      this->_current_style->_color._g, this->_current_style->_color._b, 255);
}
