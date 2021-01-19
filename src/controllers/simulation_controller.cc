#include <controllers/simulation_controller.h>

SimulationController::SimulationController(AtlasModel *atlas_model, PlayerModel *player_model,
                                           SDL_GUI::InterfaceModel *interface_model) :
    _atlas_model(atlas_model),
    _player_model(player_model),
    _interface_model(interface_model) {
}

void SimulationController::simulate() {
    this->_atlas_model->reset_for_simulation();
    Schedule::reset_next_id();
    /* sort jobs by submission time */
    std::map<int, std::vector<Job *>> jobs;
    for (Job *job: this->_atlas_model->_jobs) {
        jobs[job->_submission_time].push_back(job);
    }
    /* kickstart simulation by adding submission actions to queue */
    for (std::pair<int, std::vector<Job *>> submissions: jobs) {
        this->_atlas_model->_actions_to_do.push_back(
            new SubmissionAction(this->_atlas_model, submissions.first, submissions.second));
    }
    /* start simulation of actions */
    while (not this->_atlas_model->_actions_to_do.empty()) {
        /* sort actions */
        this->_atlas_model->_actions_to_do.sort(
            [](const SimulationAction *a, const SimulationAction *b) {
                return a->time() < b->time();
            });
        //std::cerr << "sorted actions with times: [";
        //for (auto a: this->state.actions_to_do) {
        //    std::cerr << a->time() << ", ";
        //}
        //std::cerr << "]" << std::endl;
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
