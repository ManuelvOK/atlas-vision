#pragma once

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>

/** Drawable that is a polygon in arrow shape */
class Arrow : public SDL_GUI::Drawable {
public:
    /** possible directions of the arrow */
    enum class Direction {
        UP,
        DOWN,
    };
private:
    /** polygon shape for the arrow in up and down direction */
    const std::map<Direction, std::pair<std::array<short,9>, std::array<short,9>>> _coords= {
        {
            Direction::DOWN,
            {
                {-10, 10, 10, 40, 50,   0, -50, -40, -10},
                {  0,  0, 80, 50, 60, 110,  60,  50,  80}
            },
        },
        {
            Direction::UP,
            {
                {-10, 10,  10,  40,  50,    0, -50, -40, -10},
                {  0,  0, -80, -50, -60, -110, -60, -50, -80}
            },
        },
    };

    Direction _direction;   /**< direction this arrow has */
public:
    /** Constructor */
    Arrow(std::string type = "Arrow"): Drawable(type) {}

    /**
     * Constructor
     * @param position position of arrow in parent drawable
     * @param direction direction the arrow should point to
     */
    Arrow(SDL_GUI::Position position, Direction direction = Direction::UP)
        : Drawable("Arrow", position), _direction(direction) {}

    /**
     * Constructor
     * @param type type of drawable
     * @param position position of arrow in parent drawable
     * @param direction direction the arrow should point to
     */
    Arrow(std::string type, SDL_GUI::Position position, Direction direction = Direction::UP)
        : Drawable(type, position), _direction(direction) {}

    void draw(SDL_Renderer *renderer, SDL_GUI::Position position) const override;
};
