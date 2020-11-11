#pragma once

#include <SDL_GUI/inc/models/input_model.h>
#include <SDL_GUI/inc/gui/position.h>

#include <input_keys.h>

class InputModel : public SDL_GUI::InputModel<InputKey> {
public:
    bool _player_hovered = false;
    SDL_GUI::Position _mouse_in_player = {0,0};

};
