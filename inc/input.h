#pragma once

/**
 * Struct to save input data to
 *
 * @param quit
 *   flag to determine whether quit input is given
 */
struct input {
    int quit;
};

/**
 * read input and save it to input struct
 *
 * @param input
 *   Struct to save input data to
 */
void read_input(struct input *input);
