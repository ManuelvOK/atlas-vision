#pragma once
#include <SDL2/SDL.h>

/**
 * Color Code for RGB color
 */
class RGB {
public:
    int _r = 0; /**< red value between 0 and 255 */
    int _g = 0; /**< green value between 0 and 255 */
    int _b = 0; /**< blue value between 0 and 255 */

    /**
     * Default constructor
     */
    RGB() = default;

    /**
     * Constructor for grey color
     *
     * @param grey
     *   value for r, g and b
     */
    RGB(int grey) : _r(grey), _g(grey), _b(grey) {}

    /**
     * Constructor
     *
     * @param r
     *   red value between 0 and 255
     * @param g
     *   green value between 0 and 255
     * @param b
     *   blue value between 0 and 255
     */
    RGB(int r, int g, int b) : _r(r), _g(g), _b(b) {}

    /**
     * activate color for a given alpha on a given renderer
     *
     * @param renderer
     *   renderer to activate color on
     * @param alpha
     *   alpha value for the color to activate
     */
    void activate(SDL_Renderer *renderer, int alpha = 255) const;
};
