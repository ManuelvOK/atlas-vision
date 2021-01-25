#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>
#include <SDL_GUI/inc/models/interface_model.h>

#include <models/atlas_model.h>
#include <models/player_model.h>

class SimulationController: public SDL_GUI::ControllerBase {
    AtlasModel *_atlas_model;
    PlayerModel *_player_model;
    SDL_GUI::InterfaceModel *_interface_model;
public:
    SimulationController(AtlasModel *atlas_model, PlayerModel *player_model,
                         SDL_GUI::InterfaceModel *interface_model);

    void update() override;

    void simulate();
};