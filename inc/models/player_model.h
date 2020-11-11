#pragma once

#include <SDL_GUI/inc/models/model_base.h>

class PlayerModel : public SDL_GUI::ModelBase {
    float _zoom = 1;        /**< factor to calculate pixels from units its pixels per unit*/
    float _zoom_min = 1;    /**< minimal zoom */

    int _width = 0;         /**< player window width in pixels */
    int _half_width = 0;

    int _scroll = 0;        /**< difference between player begin and middle of window in units */
public:
    int _running = 0;       /**< determines if the simulation gets played */
    int _speed = 50;        /**< this value gets added to the position every tick */
    int _position = 0;      /**< current position of player */
    int _max_position = 0;  /**< length of full simulation */



    /**
     * toggle play status
     */
    void toggle();

    /**
     * set players position to 0
     */
    void rewind();

    void skip_forwards(int value = 5);
    void skip_backwards(int value = 5);
    /**
     * set players position to a specific timestamp
     *
     * @param position
     *   timestamp to sset players position to
     */
    void set_position(int position);

    void set_position_with_click(int position);

    /**
     * trigger a player tick. That is incrementing the players position by its speed value
     */
    void tick();

    void zoom_in(float value = 1.05);
    void zoom_out(float value = 1.05);
    void set_zoom(float value);
    float zoom() const;

    void set_width(int width);

    void scroll_left(int value = 15);
    void scroll_right(int value = 15);
    void set_scroll(int value);
    int scroll() const;
    int scroll_offset() const;

    void reposition_scroll();
    void recenter();
};
