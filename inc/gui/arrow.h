#pragma once

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>

class Arrow : public SDL_GUI::Drawable {
public:
    enum class direction {
        UP,
        DOWN,
    };
private:
    const std::map<direction, std::pair<std::array<short,9>, std::array<short,9>>> _coords= {
        {
            direction::DOWN,
            {
                {-10, 10, 10, 40, 50,   0, -50, -40, -10},
                {  0,  0, 80, 50, 60, 110,  60,  50,  80}
            },
        },
        {
            direction::UP,
            {
                {-10, 10,  10,  40,  50,    0, -50, -40, -10},
                {  0,  0, -80, -50, -60, -110, -60, -50, -80}
            },
        },
    };
    direction _direction;
public:
    Arrow(): Drawable("Arrow") {}

    Arrow(SDL_GUI::Position position, direction direction = direction::UP): Drawable("Arrow", position), _direction(direction) {}

    void draw(SDL_Renderer *renderer, SDL_GUI::Position position) const override;
};
