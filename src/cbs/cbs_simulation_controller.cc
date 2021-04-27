#include <cbs/cbs_simulation_controller.h>

CbsSimulationController::CbsSimulationController(CbsSimulationModel *cbs_model,
                                                 PlayerModel *player_model,
                                                 SDL_GUI::InterfaceModel *interface_model)
    : SimulationController(cbs_model, player_model, interface_model), _cbs_model(cbs_model) {}

void CbsSimulationController::bootstrap_simulation() {
}

