#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>

#include <models/atlas_model.h>
#include <models/keyboard_input_model.h>
#include <models/mouse_input_model.h>
#include <models/player_model.h>

class PlayerController : public SDL_GUI::ControllerBase {
    PlayerModel *_player_model;
    const KeyboardInputModel *_keyboard_input_model;
    const MouseInputModel *_mouse_input_model;

    void init(const AtlasModel *atlas_model);
public:
    PlayerController(PlayerModel *player_model, const KeyboardInputModel *keyboard_input_model,
            const MouseInputModel *mouse_input_model, const AtlasModel *atlas_model);

    void update();
};
