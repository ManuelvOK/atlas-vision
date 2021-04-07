#include <gui/arrow.h>

#include <algorithm>

#include <SDL2/SDL2_gfxPrimitives.h>


const std::map<Arrow::Direction, std::pair<std::array<short,9>, std::array<short,9>>>
Arrow::_coords =
{
    {
        Direction::DOWN,
        {
            {40, 60, 60, 90, 100,  50,  0, 10, 40},
            { 0,  0, 80, 50,  60, 110, 60, 50, 80}
        },
    },
    {
        Direction::UP,
        {
            { 40,  60, 60, 90, 100, 50,  0, 10, 40},
            {110, 110, 30, 60,  50,  0, 50, 60, 30}
        },
    },
};

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

SDL_GUI::Drawable *Arrow::clone() const {
    return new Arrow(*this);
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

std::tuple<std::array<short, 9>, std::array<short, 9>> Arrow::get_drawing_coords(SDL_GUI::Position position) const {
    /* TODO: get rid of magic 9 */
    std::array<short, 9> pos_x;
    std::array<short, 9> pos_y;

    for (unsigned i = 0; i < 9; ++i) {
        /* TODO: get rid of magic 5 */
        pos_x[i] = this->_coords_x[i] + position._x;
        pos_y[i] = this->_coords_y[i] + position._y;
    }
    return std::make_tuple(pos_x, pos_y);
}

void Arrow::draw(SDL_Renderer *renderer, SDL_GUI::Position position) const {
    std::array<short, 9> pos_x;
    std::array<short, 9> pos_y;
    std::tie(pos_x, pos_y) = this->get_drawing_coords(position);
    const SDL_GUI::RGB &c = this->_style._color;
    filledPolygonRGBA(renderer, pos_x.data(), pos_y.data(), 9,
                      c._r, c._g, c._b, 255);
}

void Arrow::draw_border(SDL_Renderer *renderer, SDL_GUI::Position position) const {
    if (not this->_style._has_border) {
        return;
    }
    std::array<short, 9> pos_x;
    std::array<short, 9> pos_y;
    std::tie(pos_x, pos_y) = this->get_drawing_coords(position);
    const SDL_GUI::RGB &c = this->_style._border_color;
    polygonRGBA(renderer, pos_x.data(), pos_y.data(), 9,
                      c._r, c._g, c._b, 255);
}

unsigned Arrow::height(Direction direction) {
    auto coords_y = Arrow::_coords.at(direction).second;
    int max_y = *std::max_element(coords_y.begin(), coords_y.end());
    int min_y = *std::min_element(coords_y.begin(), coords_y.end());
    return (max_y - min_y) / 5;
}

unsigned Arrow::height() const {
    return Arrow::height(this->_direction);
}

unsigned Arrow::width(Direction direction) {
    auto coords_x = Arrow::_coords.at(direction).first;
    int max_x = *std::max_element(coords_x.begin(), coords_x.end());
    int min_x = *std::min_element(coords_x.begin(), coords_x.end());
    return (max_x - min_x) / 5;
}

unsigned Arrow::width() const {
    return Arrow::width(this->_direction);
}
