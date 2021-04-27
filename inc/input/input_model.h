#pragma once

#include <SDL_GUI/models/input_model.h>
#include <SDL_GUI/gui/position.h>

#include <config/input_config.h>

/** Model for input related data */
class InputModel : public SDL_GUI::InputModel<InputKey, InputState> {
public:
    InputModel(InputState default_input_state)
        : SDL_GUI::InputModel<InputKey, InputState>(default_input_state) {}

    /** Mouse position relative to the player rect */
    SDL_GUI::Position _mouse_in_player = {0,0};
};
