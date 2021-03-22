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
protected:
    virtual SDL_GUI::Drawable *clone() const override;

private:
    /** polygon shape for the arrow in up and down direction */
    static const std::map<Direction, std::pair<std::array<short,9>, std::array<short,9>>> _coords;
    std::array<short, 9> _coords_x;
    std::array<short, 9> _coords_y;

    Direction _direction;   /**< direction this arrow has */

    void init();

    std::tuple<std::array<short, 9>, std::array<short, 9>>
        get_drawing_coords(SDL_GUI::Position position) const;
public:
    /** Constructor */
    Arrow(std::string type = "Arrow"): Drawable(type) {}

    /**
     * Constructor
     * @param position position of arrow in parent drawable
     * @param direction direction the arrow should point to
     */
    Arrow(SDL_GUI::Position position, Direction direction = Direction::UP);

    /**
     * Constructor
     * @param type type of drawable
     * @param position position of arrow in parent drawable
     * @param direction direction the arrow should point to
     */
    Arrow(std::string type, SDL_GUI::Position position, Direction direction = Direction::UP);

    void draw(SDL_Renderer *renderer, SDL_GUI::Position position) const override;

    void draw_border(SDL_Renderer *renderer, SDL_GUI::Position position) const override;

    static unsigned height(Direction direction);

    virtual unsigned height() const override;

    static unsigned width(Direction direction);

    virtual unsigned width() const override;
};
