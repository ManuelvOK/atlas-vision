#include <controllers/simulation_controller.h>

#include <controllers/rr_core_assigner.h>

SimulationController::SimulationController(AtlasModel *atlas_model, PlayerModel *player_model,
                                           SDL_GUI::InterfaceModel *interface_model) :
    _atlas_model(atlas_model),
    _player_model(player_model),
    _interface_model(interface_model),
    _core_assigner(new RoundRobinCoreAssigner()) {
}

SimulationController::~SimulationController() {
    delete this->_core_assigner;
}

void SimulationController::simulate() {
    this->_atlas_model->reset_for_simulation();
    Schedule::reset_next_id();
    this->_core_assigner->init(this->_atlas_model->_n_cores);
    this->_core_assigner->init_assignment(this->_atlas_model->_jobs);

    /* kickstart simulation by adding submission actions to queue */
    for (Job *job: this->_atlas_model->_jobs) {
        this->_atlas_model->_actions_to_do.push_back(
            new SubmissionAction(this->_core_assigner, this->_atlas_model, job));
    }

    /* start simulation of actions */
    while (not this->_atlas_model->_actions_to_do.empty()) {
        /* sort actions */
        this->_atlas_model->_actions_to_do.sort(
            [](const SimulationAction *a, const SimulationAction *b) {
                if (a->time() == b->time()) {
                    return a->_weight < b->_weight;
                }
                return a->time() < b->time();
            });

        /* pop first action */
        SimulationAction *action = this->_atlas_model->_actions_to_do.front();
        this->_atlas_model->_actions_to_do.pop_front();

        /* update state timestamp */
        this->_atlas_model->_timestamp = action->time();

        /* execute action */
        action->action();

        /* put action to executed actions list */
        this->_atlas_model->_actions_done.push_back(action);
    }


    this->_atlas_model->_simulated = true;
    this->_atlas_model->_dirty = true;
    this->_player_model->_dirty = true;
}

void SimulationController::update() {
    if (not this->_atlas_model->_simulated) {
        this->simulate();
    }
}
