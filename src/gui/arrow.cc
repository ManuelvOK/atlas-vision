#include <gui/arrow.h>

#include <algorithm>


const std::map<Arrow::Direction, std::array<SDL_GUI::Position,9>>
Arrow::_coords =
{
    {
        Direction::DOWN,
        {
            SDL_GUI::Position{ 40,   0},
            SDL_GUI::Position{ 60,   0},
            SDL_GUI::Position{ 60,  80},
            SDL_GUI::Position{ 90,  50},
            SDL_GUI::Position{100,  60},
            SDL_GUI::Position{ 50, 110},
            SDL_GUI::Position{  0,  60},
            SDL_GUI::Position{ 10,  50},
            SDL_GUI::Position{ 40,  80}
        }
    },
    {
        Direction::UP,
        {
            SDL_GUI::Position{ 40, 110},
            SDL_GUI::Position{ 60, 110},
            SDL_GUI::Position{ 60,  30},
            SDL_GUI::Position{ 90,  60},
            SDL_GUI::Position{100,  50},
            SDL_GUI::Position{ 50,   0},
            SDL_GUI::Position{  0,  50},
            SDL_GUI::Position{ 10,  60},
            SDL_GUI::Position{ 40,  30}
        }
    },
};

Arrow::Arrow(SDL_GUI::Position position, Direction direction) :
    Polygon("Arrow", position),
    _direction(direction) {
    this->init();
}

Arrow::Arrow(std::string type, SDL_GUI::Position position, Direction direction) :
    Polygon(type, position),
    _direction(direction) {
    this->init();
}

SDL_GUI::Drawable *Arrow::clone() const {
    return new Arrow(*this);
}

void Arrow::init() {
    int min_x, max_x, min_y, max_y;
    bool first = true;
    for (SDL_GUI::Position point: this->_coords.at(this->_direction)) {
        point /= 5;
        this->add_point(point);
        if (first) {
            min_x = point._x;
            max_x = point._x;
            min_y = point._y;
            max_y = point._y;
            first = false;
        } else {
            min_x = std::min(min_x, point._x);
            max_x = std::max(max_x, point._x);
            min_y = std::min(min_y, point._y);
            max_y = std::max(max_y, point._y);
        }
    }
    this->_width = max_x - min_x;
    this->_height = max_y - min_y;
}

unsigned Arrow::height(Direction direction) {
    auto coords = Arrow::_coords.at(direction);
    SDL_GUI::Position max = *std::max_element(coords.begin(), coords.end(),
        [](const SDL_GUI::Position &a, const SDL_GUI::Position &b){
            return a._y < b._y;
        });
    SDL_GUI::Position min = *std::min_element(coords.begin(), coords.end(),
        [](const SDL_GUI::Position &a, const SDL_GUI::Position &b){
            return a._y < b._y;
        });
    return (max._y - min._y) / 5;
}

unsigned Arrow::height() const {
    return Arrow::height(this->_direction);
}

unsigned Arrow::width(Direction direction) {
    auto coords = Arrow::_coords.at(direction);
    SDL_GUI::Position max = *std::max_element(coords.begin(), coords.end(),
        [](const SDL_GUI::Position &a, const SDL_GUI::Position &b){
            return a._x < b._x;
        });
    SDL_GUI::Position min = *std::min_element(coords.begin(), coords.end(),
        [](const SDL_GUI::Position &a, const SDL_GUI::Position &b){
            return a._x < b._x;
        });
    return (max._x - min._x) / 5;
}

unsigned Arrow::width() const {
    return Arrow::width(this->_direction);
}
