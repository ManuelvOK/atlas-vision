#pragma once

#include <SDL_GUI/controllers/controller_base.h>
#include <SDL_GUI/models/interface_model.h>

#include <controllers/core_assigner.h>
#include <models/atlas_model.h>
#include <models/player_model.h>

class SimulationController: public SDL_GUI::ControllerBase {
protected:
    SimulationModel *_simulation_model;
    PlayerModel *_player_model;
    SDL_GUI::InterfaceModel *_interface_model;
    CoreAssigner *_core_assigner;

    virtual void bootstrap_simulation() = 0;
public:
    SimulationController(SimulationModel *simulation_model, PlayerModel *player_model,
                         SDL_GUI::InterfaceModel *interface_model);

    ~SimulationController();

    void update() override;

    void simulate();
};
