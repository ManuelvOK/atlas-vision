#pragma once

#include <SDL_GUI/inc/models/model_base.h>

class PlayerModel : public SDL_GUI::ModelBase {
public:
    int _running = 0;       /**< determines if the simulation gets played */
    int _speed = 200;       /**< this value gets added to the position every tick */
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

    /**
     * set players position to a specific timestamp
     *
     * @param position
     *   timestamp to sset players position to
     */
    void set(int position);

    /**
     * trigger a player tick. That is incrementing the players position by its speed value
     */
    void tick();
};
