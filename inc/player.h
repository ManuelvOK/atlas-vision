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
    float speed = 0.04f;
    /**
     * current position of player
     */
    float position = 0;
    /**
     * length of full simulation
     */
    float max_position = 0;

    void init(const Model *model);

    void toggle();
    void rewind();
    void set(float position);
    void tick();

};
