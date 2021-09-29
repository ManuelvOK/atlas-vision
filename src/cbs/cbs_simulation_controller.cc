#include <cbs/cbs_simulation_controller.h>

#include <fstream>

#include <cbs/cbs_simulation_action.h>

CbsSimulationController::CbsSimulationController(SDL_GUI::ApplicationBase *application,
                                                 CbsSimulationModel *cbs_model,
                                                 PlayerModel *player_model)
    : SimulationController(application, cbs_model, player_model),
      _cbs_model(cbs_model) {}

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
    for (unsigned i = 0; i < this->_cbs_model->_n_cores; ++i) {
        this->_cbs_model->_active_schedules[i] = nullptr;
    }
}

void CbsSimulationController::write_back(std::string output_file) const {
    std::ofstream f_out(output_file);
    if (not f_out.is_open()) {
        std::cerr << "Error while opening output file." << std::endl;
        return;
    }

    std::stringstream ss;
    for (CbsJob *job: this->_simulation_model->specific_jobs()) {
        ss << job->to_string();
    }

    for (const auto &[_,cbs]: this->_cbs_model->_servers) {
        ss << cbs.to_string();
    }

    f_out << ss.str();
    f_out.close();
}

std::vector<std::string> CbsSimulationController::check_simulation() const {
    std::vector<std::string> errors;

    return errors;
}

