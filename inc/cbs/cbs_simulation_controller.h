#pragma once

#include <cbs/cbs_simulation_model.h>
#include <simulation/simulation_controller.h>

class CbsSimulationController : public SimulationController<CbsSchedule, CbsJob> {
    CbsSimulationModel *_cbs_model;

    void bootstrap_simulation() override;
public:
    CbsSimulationController(CbsSimulationModel *cbs_model, PlayerModel *player_model,
                            SDL_GUI::InterfaceModel *interface_model);
};
