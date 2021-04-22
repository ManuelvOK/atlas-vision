#pragma once

#include <controllers/simulation_controller.h>
#include <models/atlas_model.h>

class AtlasSimulationController : public SimulationController {
    AtlasModel *_atlas_model;

    void bootstrap_simulation() override;
public:
    AtlasSimulationController(AtlasModel *atlas_model, PlayerModel *player_model,
                              SDL_GUI::InterfaceModel *interface_model);

};
