#pragma once

#include <SDL_GUI/application.h>
#include <SDL_GUI/controllers/controller_base.h>
#include <SDL_GUI/models/interface_model.h>

#include <atlas/atlas_simulation_model.h>
#include <player/player_model.h>
#include <simulation/rr_core_assigner.h>



template <typename S, typename J>
class SimulationController: public SDL_GUI::ControllerBase {
protected:
    SDL_GUI::ApplicationBase *_application;             /**< The application */
    SimulationModel<S, J> *_simulation_model;
    PlayerModel *_player_model;
    SDL_GUI::InterfaceModel *_interface_model;
    CoreAssigner *_core_assigner;

    virtual void bootstrap_simulation() = 0;

    virtual void write_back(std::string output_file) const = 0;

    virtual std::vector<std::string> check_simulation() const = 0;
public:
    SimulationController(SDL_GUI::ApplicationBase *application,
                         SimulationModel<S, J> *simulation_model,
                         PlayerModel *player_model,
                         SDL_GUI::InterfaceModel *interface_model)
        : _application(application),
          _simulation_model(simulation_model),
          _player_model(player_model),
          _interface_model(interface_model),
          _core_assigner(new RoundRobinCoreAssigner()) {}

    ~SimulationController() {
        delete this->_core_assigner;
    }

    void update() override {
        if (not this->_simulation_model->_simulated) {
            this->simulate();
        }
    }

    void simulate() {
        this->_simulation_model->reset_for_simulation();
        BaseSchedule::reset_next_id();
        this->_core_assigner->init(this->_simulation_model->_n_cores);
        this->_core_assigner->init_assignment(this->_simulation_model->jobs());

        this->bootstrap_simulation();

        /* start simulation of actions */
        while (not this->_simulation_model->_actions_to_do.empty()) {
            /* sort actions */
            this->_simulation_model->_actions_to_do.sort(
                [](const SimulationAction *a, const SimulationAction *b) {
                    /* sort unsuccessful actions to the end */
                    if (a->_success != b->_success) {
                        return a->_success > b->_success;
                    }

                    /* sort different action types regarding their weight */
                    if (a->time() == b->time()) {
                        return a->_weight < b->_weight;
                    }

                    /* regular sort regarding time */
                    return a->time() < b->time();
                });

            /* reset sucess on all actions */
            for (SimulationAction *action: this->_simulation_model->_actions_to_do) {
                action->_success = true;
            }

            /* pop first action */
            SimulationAction *action = this->_simulation_model->_actions_to_do.front();

            /* update state timestamp */
            this->_simulation_model->_timestamp = std::max(action->time(),
                                                           this->_simulation_model->_timestamp);

            /* execute action */
            action->execute();

            if (not action->_success) {
                continue;
            }
            /* put action to executed actions list */
            this->_simulation_model->_actions_to_do.pop_front();
            this->_simulation_model->_actions_done.push_back(action);
        }


        this->_simulation_model->_simulated = true;
        this->_simulation_model->_dirty = true;
        this->_player_model->_dirty = true;
        if (this->_simulation_model->_only_simulation) {
            this->_application->_is_running = false;
        }

        /* check simulation for sanity */
        std::vector<std::string> errors = this->check_simulation();
        for (std::string error: errors) {
            std::cerr << error << std::endl;
        }

        /* write back simulated data */
        if (this->_simulation_model->_output_file != "") {
            this->write_back(this->_simulation_model->_output_file);
        }
    }
};
