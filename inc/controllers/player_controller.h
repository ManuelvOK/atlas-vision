#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>
#include <SDL_GUI/inc/models/keyboard_input_model.h>

#include <input_keys.h>
#include <models/atlas_model.h>
#include <models/interface_model.h>
#include <models/mouse_input_model.h>
#include <models/player_model.h>
#include <models/player_view_model.h>

class PlayerController : public SDL_GUI::ControllerBase {
    PlayerModel *_player_model;
    PlayerViewModel *_player_view_model;
    const SDL_GUI::KeyboardInputModel<InputKey> *_keyboard_input_model;
    const MouseInputModel *_mouse_input_model;
    InterfaceModel *_interface_model;

    void init(const AtlasModel *atlas_model);

    void evaluate_input();

    void create_sub_and_dl_arrows(const AtlasModel *atlas_model);
public:
    PlayerController(PlayerModel *player_model, PlayerViewModel *player_view_model, const SDL_GUI::KeyboardInputModel<InputKey> *keyboard_input_model,
            const MouseInputModel *mouse_input_model, const AtlasModel *atlas_model, InterfaceModel *interface_model);

    void update();
};
