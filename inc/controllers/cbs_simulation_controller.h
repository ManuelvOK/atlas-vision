#pragma once

#include <controllers/simulation_controller.h>
#include <models/cbs_model.h>

class CbsSimulationController : public SimulationController {
    CbsModel *_cbs_model;

    void bootstrap_simulation() override;
public:
    CbsSimulationController(CbsModel *cbs_model, PlayerModel *player_model,
                            SDL_GUI::InterfaceModel *interface_model);
};
