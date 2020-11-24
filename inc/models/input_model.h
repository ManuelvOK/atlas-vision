#pragma once

#include <SDL_GUI/inc/models/input_model.h>
#include <SDL_GUI/inc/gui/position.h>

#include <input_keys.h>

/** Model for input related data */
class InputModel : public SDL_GUI::InputModel<InputKey> {
public:
    /** Flag that determines whether the mouse is currenty over the player rect */
    bool _player_hovered = false;

    /** Mouse position relative to the player rect */
    SDL_GUI::Position _mouse_in_player = {0,0};
};
