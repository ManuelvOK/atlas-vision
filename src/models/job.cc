#include <models/job.h>

#include <iostream>
#include <sstream>

#include <models/atlas_model.h>

void Job::add_known_dependency(Job *job) {
    this->_known_dependencies.push_back(job);
    job->_known_dependees.push_back(this);
}

void Job::add_unknown_dependency(Job *job) {
    this->_unknown_dependencies.push_back(job);
    job->_unknown_dependees.push_back(this);
}

std::vector<Job *> Job::known_dependencies() {
    return this->_known_dependencies;
}

std::vector<Job *> Job::unknown_dependencies() {
    return this->_unknown_dependencies;
}

std::vector<Job *> Job::dependencies() {
    std::vector<Job *> ret = this->_known_dependencies;
    ret.insert(ret.end(), this->_unknown_dependencies.begin(), this->_unknown_dependencies.end());
    return ret;
}

std::vector<Job *> Job::known_dependees() {
    return this->_known_dependees;
}

std::vector<Job *> Job::unknown_dependees() {
    return this->_unknown_dependees;
}

std::vector<Job *> Job::dependees() {
    std::vector<Job *> ret = this->_known_dependees;
    ret.insert(ret.end(), this->_unknown_dependees.begin(), this->_unknown_dependees.end());
    return ret;
}

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
    if (not this->all_dependencies_finished(timestamp)) {
        return 0;
    }
    int time_executed = 0;
    for (Schedule *s: this->_schedules) {
        ScheduleData data = s->get_data_at_time(timestamp);
        if (not data._does_execute) {
            continue;
        }
        int value = std::min(data._execution_time, std::max(0, timestamp - data._begin));
        if (data._scheduler == SchedulerType::CFS) {
            value /= this->_atlas_model->_cfs_factor;
        }
        time_executed += value;
    }
    return time_executed;
}

bool Job::all_dependencies_finished(int timestamp) const {
    for (const Job *j: this->_known_dependencies) {
        if (j->finished(timestamp)) {
            continue;
        }
        return false;
    }
    for (const Job *j: this->_unknown_dependencies) {
        if (j->finished(timestamp)) {
            continue;
        }
        return false;
    }
    return true;
}

bool Job::finished(int timestamp) const {
    return this->execution_time_left(timestamp) <= 0;
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

bool Job::depends_on(const Job *job) const {
    return std::find(this->_known_dependencies.begin(), this->_known_dependencies.end(), job)
           != this->_known_dependencies.end();
}

Schedule *Job::schedule_at_time(int timestamp) {
    for (Schedule *s: this->_schedules) {
        ScheduleData data = s->get_data_at_time(timestamp);
        if (data._begin >= timestamp && data.end() <= timestamp) {
            return s;
        }
    }
    return nullptr;
}

void Job::set_atlas_schedule(AtlasSchedule *schedule) {
    this->_atlas_schedule = schedule;
    this->_schedules.push_back(schedule);
}
