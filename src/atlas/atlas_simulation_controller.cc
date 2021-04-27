#include <atlas/atlas_simulation_controller.h>

#include <atlas/atlas_simulation_action.h>

AtlasSimulationController::AtlasSimulationController(AtlasSimulationModel *atlas_model,
                                                     PlayerModel *player_model,
                                                     SDL_GUI::InterfaceModel *interface_model)
    : SimulationController(atlas_model, player_model, interface_model), _atlas_model(atlas_model) {}

void AtlasSimulationController::bootstrap_simulation() {
    /* kickstart simulation by adding submission actions to queue */
    for (AtlasJob *job: this->_simulation_model->_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new AtlasSubmissionAction(this->_core_assigner, this->_atlas_model, job));
    }
}
