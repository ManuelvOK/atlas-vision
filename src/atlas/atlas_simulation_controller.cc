#include <atlas/atlas_simulation_controller.h>

#include <fstream>

#include <atlas/atlas_simulation_action.h>

AtlasSimulationController::AtlasSimulationController(SDL_GUI::ApplicationBase *application,
                                                     AtlasSimulationModel *atlas_model,
                                                     PlayerModel *player_model,
                                                     SDL_GUI::InterfaceModel *interface_model)
    : SimulationController(application, atlas_model, player_model, interface_model),
      _atlas_model(atlas_model) {}

void AtlasSimulationController::bootstrap_simulation() {
    /* kickstart simulation by adding submission actions to queue */
    for (AtlasJob *job: this->_simulation_model->_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new AtlasSubmissionAction(this->_core_assigner, this->_atlas_model, job));
    }
}

void AtlasSimulationController::write_back(std::string output_file) {
    std::ofstream f_out(output_file);

    if (not f_out.is_open()) {
        std::cerr << "Error while opening output file." << std::endl;
        return;
    }

    std::stringstream ss;
    for (AtlasJob *job: this->_simulation_model->specific_jobs()) {
        ss << job->to_string();
    }

    f_out << ss.str();
    f_out.close();
}
