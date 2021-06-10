#include <atlas/atlas_simulation_controller.h>

#include <fstream>

#include <atlas/atlas_simulation_action.h>

AtlasSimulationController::AtlasSimulationController(SDL_GUI::ApplicationBase *application,
                                                     AtlasSimulationModel *atlas_model,
                                                     PlayerModel *player_model)
    : SimulationController(application, atlas_model, player_model),
      _atlas_model(atlas_model) {}

void AtlasSimulationController::bootstrap_simulation() {
    /* kickstart simulation by adding submission actions to queue */
    for (AtlasJob *job: this->_simulation_model->_jobs) {
        this->_simulation_model->_actions_to_do.push_back(
            new AtlasSubmissionAction(this->_core_assigner, this->_atlas_model, job));
    }
}

void AtlasSimulationController::write_back(std::string output_file) const {
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

std::vector<std::string> AtlasSimulationController::check_simulation() const {
    std::vector<std::string> errors;
    return errors;

    std::vector<const BaseAtlasSchedule *> schedules;

    for (const AtlasJob *job: this->_simulation_model->specific_jobs()) {
        unsigned total_execution_time = 0;
        for (const BaseAtlasSchedule *schedule: job->_schedules) {
            AtlasScheduleData data = schedule->last_data();

            if (data._scheduler == AtlasSchedulerType::CFS) {
                total_execution_time += data._execution_time / this->_atlas_model->_cfs_factor;
            } else {
                total_execution_time += data._execution_time;
            }

            schedules.push_back(schedule);
        }

        /* check if execution time has fully been run */
        if (total_execution_time != job->_execution_time) {
            std::stringstream ss;
            ss << "Simulation Error: Job " << job->_id << " has execution time of "
               << job->_execution_time << " but did run " << total_execution_time << ".";
            errors.push_back(ss.str());
        }
    }

    /* check if schedules do overlap */
    for (const BaseAtlasSchedule *schedule: schedules) {
        for (const BaseAtlasSchedule *schedule_against: schedules) {
            /* same schedule */
            if (schedule == schedule_against) {
                continue;
            }
            /* different cores */
            if (schedule->_core != schedule_against->_core) {
                continue;
            }
            AtlasScheduleData data = schedule->last_data();
            AtlasScheduleData data_against = schedule_against->last_data();
            /* schedules do not execute */
            if (data._execution_time == 0 or data_against._execution_time == 0) {
                continue;
            }
            /* schedule ends before other begins */
            if (data.end() <= data_against._begin) {
                continue;
            }
            /* schedule begins after other ends */
            if (data._begin >= data_against.end()) {
                continue;
            }
            /* there's overlap */
            std::stringstream ss;
            ss << "Simulation Error: Schedules overlapping on core " << schedule->_core << ": "
               << std::endl << "\t<Job " << schedule->job()->_id << ", " << data._begin << " - "
               << data.end() << ">"
               << std::endl << "\t<Job " << schedule_against->job()->_id << ", "
               << data_against._begin << " - " << data_against.end() << ">";
            errors.push_back(ss.str());
        }
    }

    return errors;
}
