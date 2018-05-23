#pragma once

#include <vector>

/**
 * Representation of the simulation player
 */
class Model;
class Player {
public:
    /**
     * determines if the simulation gets played
     */
    int running = 0;
    /**
     * this value gets added to the position every tick
     */
    int speed = 200;
    /**
     * current position of player
     */
    int position = 0;
    /**
     * length of full simulation
     */
    int max_position = 0;

    void init(const Model *model);

    void toggle();
    void rewind();
    void set(int position);
    void tick();

};
