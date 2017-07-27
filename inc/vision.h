#pragma once

#include <SDL2/SDL.h>

/**
 * initialise graphics
 */
void init_graphics();

/**
 * render a frame from a given state.
 *
 * @param state
 *   application state to render
 */
void render_vision(const struct state *state);

/**
 * calculate positions of boxes for vision.
 *
 * This is for performance reasons. Since the jobs don't change during the execution of the
 * application positioning can be precomputed.
 *
 * @param state
 *   application state to precompute vision for
 */
void calculate_vision(const struct state *state);
