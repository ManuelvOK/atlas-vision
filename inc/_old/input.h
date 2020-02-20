#pragma once
#include <view/view.h>

class View;
class Model;
/**
 * Struct to save input data to
 *
 * @param quit
 *   flag to determine whether quit input is given
 */
struct input {
    int mouse_position_x;
    int mouse_position_y;
    int mouse_offset_x;
    int mouse_offset_y;
    int quit;
    struct {
        int toggle_play;
        int rewind;
        int position;
        int offset;
    } player;
    float rescale;
    struct {
        int changed;
        int width;
        int height;
    } window;

};

/**
 * read input and save it to input struct
 *
 * @param input
 *   Struct to save input data to
 */
void read_input(struct input *input, const View *view);