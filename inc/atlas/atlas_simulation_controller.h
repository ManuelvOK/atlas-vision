#pragma once

#include <SDL_GUI/application.h>

#include <simulation/simulation_controller.h>
#include <atlas/atlas_simulation_model.h>

class AtlasSimulationController : public SimulationController<BaseAtlasSchedule, AtlasJob> {
    AtlasSimulationModel *_atlas_model;

    void bootstrap_simulation() override;
    void write_back(std::string output_file) override;
public:
    AtlasSimulationController(SDL_GUI::ApplicationBase *application,
                              AtlasSimulationModel *atlas_model,
                              PlayerModel *player_model,
                              SDL_GUI::InterfaceModel *interface_model);

};
