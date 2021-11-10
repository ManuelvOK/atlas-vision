#include <grub/grub_simulation_controller.h>

#include <fstream>

#include <grub/grub_simulation_action.h>

GrubSimulationController::GrubSimulationController(SDL_GUI::ApplicationBase *application,
                                                 GrubSimulationModel *grub_model,
                                                 PlayerModel *player_model)
    : SimulationController(application, grub_model, player_model),
      _grub_model(grub_model) {}

void GrubSimulationController::bootstrap_simulation() {
    /* submission for hard realtime jobs */
    for (HardGrubJob *job: this->_grub_model->_hard_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new GrubSubmissionAction(this->_core_assigner, this->_grub_model, job));
    }

    /* submission for soft realtime jobs */
    for (SoftGrubJob *job: this->_grub_model->_soft_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new GrubSubmissionAction(this->_core_assigner, this->_grub_model, job));
    }
    for (unsigned i = 0; i < this->_grub_model->_n_cores; ++i) {
        this->_grub_model->_active_schedules[i] = nullptr;
    }
}

void GrubSimulationController::write_back(std::string output_file) const {
    std::ofstream f_out(output_file);
    if (not f_out.is_open()) {
        std::cerr << "Error while opening output file." << std::endl;
        return;
    }

    std::stringstream ss;
    for (GrubJob *job: this->_simulation_model->specific_jobs()) {
        ss << job->to_string();
    }

    for (const auto &[_,cbs]: this->_grub_model->_servers) {
        ss << cbs.to_string();
    }

    f_out << ss.str();
    f_out.close();
}

std::vector<std::string> GrubSimulationController::check_simulation() const {
    std::vector<std::string> errors;

    return errors;
}
