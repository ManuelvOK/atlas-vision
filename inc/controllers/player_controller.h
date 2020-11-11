#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>
#include <SDL_GUI/inc/models/interface_model.h>

#include <input_keys.h>
#include <models/atlas_model.h>
#include <models/input_model.h>
#include <models/interface_model.h>
#include <models/player_model.h>
#include <models/player_view_model.h>


class PlayerController : public SDL_GUI::ControllerBase {
    PlayerModel *_player_model;
    PlayerViewModel *_player_view_model;
    InputModel *_input_model;
    InterfaceModel *_interface_model;
    SDL_GUI::InterfaceModel *_default_interface_model;

    bool _dragging = false;

    void init(const AtlasModel *atlas_model);

    void evaluate_input();

    void drag();
public:
    PlayerController(PlayerModel *player_model, PlayerViewModel *player_view_model, InputModel *input_model, const AtlasModel *atlas_model, InterfaceModel *interface_model, SDL_GUI::InterfaceModel *default_interface_model);

    void update();
};
