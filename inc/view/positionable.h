#pragma once

/**
 * 2 dimensional point
 */
class Position {
public:
    int _x = 0; /**< horizontal position */
    int _y = 0; /**< vertical position */

    /**
     * Constructor
     * Position (0,0)
     */
    Position() {}

    /**
     * Constructor
     *
     * @param x
     *   horizontal position
     * @param y
     *   vertical position
     */
    Position(int x, int y) : _x(x), _y(y) {}

    Position operator+(Position &p) {
        return Position(this->_x + p._x, this->_y + p._y);
    }
};

/**
 * Objects consisting out of a position, width and height
 */
class Positionable {
public:
    Position _position; /**< objects position */
    int _width = 0; /**< objects width */
    int _height = 0; /**< objects height */

    /**
     * Constructor
     * Position (0,0), width 0, height 0
     */
    Positionable() : _position() {}

    /**
     * Constructor
     *
     * @param x
     *   horizontal position
     * @param y
     *   vertical position
     * @param width
     *   width
     * @param height
     *   height
     */
    Positionable(int x, int y, int width, int height) : _position(x, y), _width(width), _height(height) {}
};
