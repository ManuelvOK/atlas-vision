#include <cbs/cbs_simulation_controller.h>

#include <cbs/cbs_simulation_action.h>

CbsSimulationController::CbsSimulationController(CbsSimulationModel *cbs_model,
                                                 PlayerModel *player_model,
                                                 SDL_GUI::InterfaceModel *interface_model)
    : SimulationController(cbs_model, player_model, interface_model), _cbs_model(cbs_model) {}

void CbsSimulationController::bootstrap_simulation() {
    /* submission for hard realtime jobs */
    for (HardRtJob *job: this->_cbs_model->_hard_rt_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new CbsSubmissionAction(this->_core_assigner, this->_cbs_model, job));
    }

    /* submission for soft realtime jobs */
    for (SoftRtJob *job: this->_cbs_model->_soft_rt_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new CbsSubmissionAction(this->_core_assigner, this->_cbs_model, job));
    }
}

