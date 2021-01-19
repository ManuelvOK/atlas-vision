#include <models/job.h>

#include <iostream>
#include <sstream>

#include <models/atlas_model.h>

int Job::calculate_dependency_level() {
    /* only calculate if already set */
    if (this->_dependency_level >= 0) {
        return this->_dependency_level;
    }
    if (this->_known_dependencies.size() == 0 && this->_unknown_dependencies.size() == 0) {
        this->_dependency_level = 0;
        return 0;
    }
    this->_dependency_level = 1;
    for (Job *j: this->_known_dependencies) {
        this->_dependency_level = std::max(this->_dependency_level,
                                           1 + j->calculate_dependency_level());
    }
    for (Job *j: this->_unknown_dependencies) {
        this->_dependency_level = std::max(this->_dependency_level,
                                           1 + j->calculate_dependency_level());
    }
    return this->_dependency_level;
}

int Job::execution_time_left(int timestamp) const {
    return this->_execution_time - this->time_executed(timestamp);
}

int Job::estimated_execution_time_left(int timestamp) const {
    return this->_execution_time_estimate - this->time_executed(timestamp);
}

int Job::time_executed(int timestamp) const {
    int time_executed = 0;
    for (Schedule *s: this->_schedules) {
        ScheduleData data = s->get_data_at_time(timestamp);
        int value = std::min(data._execution_time, std::max(0, timestamp - data._begin));
        if (data._scheduler == SchedulerType::CFS) {
            value /= this->_atlas_model->_cfs_factor;
        }
        time_executed += value;
    }
    return time_executed;
}

std::string Job::to_string() const {
    std::stringstream ss;
    ss << "j " << this->_id
       << " " << this->_deadline
       << " " << this->_execution_time_estimate
       << " " << this->_execution_time
       << " " << this->_submission_time
       << " # id=" << this->_id
       << std::endl;

    for (Job *j: this->_known_dependencies) {
        ss << "d k "
            << this->_id
            << " " << j->_id
            << std::endl;
    }
    for (Job *j: this->_unknown_dependencies) {
        ss << "d u "
            << this->_id
            << " " << j->_id
            << std::endl;
    }
    return ss.str();
}
