#pragma once

#include <SDL_GUI/gui/primitives/polygon.h>
#include <SDL_GUI/gui/rgb.h>

/** Drawable that is a polygon in arrow shape */
class Arrow : public SDL_GUI::Polygon {
public:
    /** possible directions of the arrow */
    enum class Direction {
        UP,
        DOWN,
    };
private:
    /** polygon shape for the arrow in up and down direction */
    static const std::map<Direction, std::array<SDL_GUI::Position,9>> _coords;

    Direction _direction;   /**< direction this arrow has */

    void init();

protected:

    /**
     * Constructor
     * @param type type of drawable
     * @param position position of arrow in parent drawable
     * @param direction direction the arrow should point to
     */
    Arrow(std::string type, SDL_GUI::Position position, Direction direction = Direction::UP);

    virtual SDL_GUI::Drawable *clone() const override;

public:
    /** Constructor */
    Arrow(std::string type = "Arrow"): Polygon(type) {}

    /**
     * Constructor
     * @param position position of arrow in parent drawable
     * @param direction direction the arrow should point to
     */
    Arrow(SDL_GUI::Position position, Direction direction = Direction::UP);


    static unsigned height(Direction direction);

    virtual unsigned height() const override;

    static unsigned width(Direction direction);

    virtual unsigned width() const override;

    Direction direction() const;
};
