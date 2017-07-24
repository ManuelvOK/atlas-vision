#pragma once

#include <SDL2/SDL.h>

void init_graphics();
void render_vision(const struct state *state);
void calculate_vision(const struct state *state);
