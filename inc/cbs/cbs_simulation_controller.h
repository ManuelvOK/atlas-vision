#pragma once

#include <SDL_GUI/application.h>

#include <cbs/cbs_simulation_model.h>
#include <simulation/simulation_controller.h>

class CbsSimulationController : public SimulationController<CbsSchedule, CbsJob> {
    CbsSimulationModel *_cbs_model;

    void bootstrap_simulation() override;
    void write_back(std::string output_file) const override;
    std::vector<std::string> check_simulation() const override;
public:
    CbsSimulationController(SDL_GUI::ApplicationBase *application,
                            CbsSimulationModel *cbs_model,
                            PlayerModel *player_model,
                            SDL_GUI::InterfaceModel *interface_model);
};
