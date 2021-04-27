#pragma once

#include <simulation/simulation_controller.h>
#include <atlas/atlas_simulation_model.h>

class AtlasSimulationController : public SimulationController<BaseAtlasSchedule, AtlasJob> {
    AtlasSimulationModel *_atlas_model;

    void bootstrap_simulation() override;
public:
    AtlasSimulationController(AtlasSimulationModel *atlas_model, PlayerModel *player_model,
                              SDL_GUI::InterfaceModel *interface_model);

};
