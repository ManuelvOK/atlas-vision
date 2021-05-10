#pragma once

#include <SDL_GUI/models/model_base.h>

/**
 * Model for all schedule player related data.
 */
class PlayerModel : public SDL_GUI::ModelBase {
    float _zoom = 1;        /**< factor to calculate pixels from units its pixels per unit*/
    float _zoom_min = 1;    /**< minimal zoom */
    unsigned _width = 0;         /**< player window width in pixels */
    unsigned _half_width = 0;
    int _scroll = 0;        /**< difference between player begin and middle of window in units */
public:
    bool _running = false;       /**< determines if the simulation gets played */
    unsigned _speed = 50;        /**< this value gets added to the position every tick */
    unsigned _position_before = 0;
    unsigned _position = 0;      /**< current position of player */
    unsigned _max_position = 0;  /**< length of full simulation */
    bool _dirty = true;

    /** toggle play status */
    void toggle();

    /** set players position to 0 */
    void rewind();

    /**
     * increment player position by a certain value
     * @param value value to increment payers position by
     */
    void skip_forwards(unsigned value = 5);

    /**
     * decrement player position by a certain value
     * @param value value to decrement payers position by
     */
    void skip_backwards(unsigned value = 5);

    /**
     * set players position to a specific timestamp
     * @param position timestamp to sset players position to
     */
    void set_position(unsigned position);

    /**
     * set players position based on a mouse click that happened
     * @param position x value of relative position of mouse click inside player
     */
    void set_position_with_click(unsigned position);

    /** trigger a player tick. That is incrementing the players position by its speed value */
    void tick();

    /**
     * multiply zoom value by a certain value
     * @param value value to multiply zoom value with
     */
    void zoom_in(float value = 1.05);

    /**
     * multiply zoom value by a certain value while focussing a position
     * @param position player position to focus
     * @param value value to multiply zoom value with
     */
    void zoom_in_pos(unsigned position, float value = 1.05);

    /**
     * divide zoom value by a certain value
     * @param value value to devide zoom value by
     */
    void zoom_out(float value = 1.05);

    /**
     * divide zoom value by a certain value while focussing a position
     * @param position player position to focus
     * @param value value to divide zoom value with
     */
    void zoom_out_pos(unsigned position, float value = 1.05);

    /**
     * setter for the zoom value. This does bounds checking.
     * @param value value to set zoom to
     */
    void set_zoom(float value);

    /*
     * multiply zoom value with focussing a position. This does bounds checking.
     * @param position player position to focus
     * @param value value to multiply zoom with
     */
    void zoom_pos(unsigned position, float value);

    /**
     * Getter for the curent zoom value
     * @return current zoom value
     */
    float zoom() const;

    /**
     * Set the width of the player rect.
     * This will recalculate zoom bounds and reset the zoom.
     * @param width width f the player rect in px
     */
    void set_width(unsigned width);

    /**
     * move player to the left by a certain value. This does bounds checking
     * @param value value to move the player
     */
    void scroll_left(unsigned value = 15);

    /**
     * move player to the right by a certain value. This does bounds checking
     * @param value value to move the player
     */
    void scroll_right(unsigned value = 15);


    /**
     * Set the scroll position of the player to a certain value. This does bound checking
     * @param value value to set the players scroll position to
     */
    void set_scroll(int value);


    /**
     * getter for the players scroll position. That is the difference between the begin of the
     * player and the middle of the players rect in units.
     * @return players scroll position
     */
    int scroll() const;

    /**
     * getter for the players scroll offset. That is the number of pixels the player is moved
     * through scrolling
     * @return scroll offset
     */
    int scroll_offset() const;

    unsigned position_from_coord(unsigned x) const;

    /** redo bounds checking */
    void reposition_scroll();

    /** set the scroll position so that the current player position is at the exact center */
    void recenter();
};
