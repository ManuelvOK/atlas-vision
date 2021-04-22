#include <models/simulation_model.h>


bool compare_schedules(const Schedule *a, const Schedule *b) {
    int begin_a = a->last_data()._begin;
    int begin_b = b->last_data()._begin;
    if (begin_a == begin_b) {
        return a->_id < b->_id;
    }
    return begin_a < begin_b;
}

SimulationModel::SimulationModel() : _schedules(compare_schedules) {}

void SimulationModel::add_message(int timestamp, std::string text, std::set<int> jobs) {
    Message *message = new Message(timestamp, text, jobs);
    this->_messages.push_back(message);
    std::cerr << timestamp << ": " << text << std::endl;
}

const Schedule *SimulationModel::active_schedule(unsigned core, int timestamp) const {
    for (const Schedule *s: this->_schedules) {
        if (s->is_active_at_time(timestamp) and s->_core == core) {
            return s;
        }
    }
    return nullptr;
}


void SimulationModel::resort_schedules() {
    auto old_schedules = this->_schedules;
    this->_schedules.clear();
    for (Schedule *s: old_schedules) {
        this->_schedules.insert(s);
    }
}

void SimulationModel::reset_for_simulation() {
    this->_timestamp = 0;

    for (Schedule *schedule: this->_schedules) {
        delete schedule;
    }
    this->_schedules.clear();

    for (Job *job: this->_jobs) {
        job->_schedules.clear();
    }
    for (SimulationAction *action: this->_actions_done) {
        delete action;
    }
    this->_actions_done.clear();
}
