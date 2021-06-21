#include <atlas/atlas_job.h>

#include <atlas/atlas_simulation_model.h>

void AtlasJob::add_known_dependency(AtlasJob *job) {
    this->_known_dependencies.push_back(job);
    job->_known_dependees.push_back(this);
}

void AtlasJob::add_unknown_dependency(AtlasJob *job) {
    this->_unknown_dependencies.push_back(job);
    job->_unknown_dependees.push_back(this);
}

std::vector<AtlasJob *> AtlasJob::known_dependencies() {
    return this->_known_dependencies;
}

std::vector<AtlasJob *> AtlasJob::not_finished_known_dependencies(int timestamp) {
    std::vector<AtlasJob *> ret;
    std::copy_if(this->_known_dependencies.begin(), this->_known_dependencies.end(),
                 std::back_inserter(ret),
                 [timestamp] (AtlasJob *job) {
                     return job->execution_time_left(timestamp) > 0;
                 });
    return ret;
}

std::vector<AtlasJob *> AtlasJob::unknown_dependencies() {
    return this->_unknown_dependencies;
}

std::vector<AtlasJob *> AtlasJob::dependencies() {
    std::vector<AtlasJob *> ret = this->_known_dependencies;
    ret.insert(ret.end(), this->_unknown_dependencies.begin(), this->_unknown_dependencies.end());
    return ret;
}

std::vector<AtlasJob *> AtlasJob::known_dependees() {
    return this->_known_dependees;
}

std::vector<AtlasJob *> AtlasJob::unknown_dependees() {
    return this->_unknown_dependees;
}

std::vector<AtlasJob *> AtlasJob::dependees() {
    std::vector<AtlasJob *> ret = this->_known_dependees;
    ret.insert(ret.end(), this->_unknown_dependees.begin(), this->_unknown_dependees.end());
    return ret;
}

unsigned AtlasJob::calculate_dependency_level() {
    /* only calculate if already set */
    if (this->_dependency_level_calculated) {
        return this->_dependency_level;
    }

    if (this->_known_dependencies.size() == 0 && this->_unknown_dependencies.size() == 0) {
        this->_dependency_level = 0;
        this->_dependency_level_calculated = true;
        return 0;
    }
    this->_dependency_level = 1;
    for (AtlasJob *j: this->_known_dependencies) {
        this->_dependency_level = std::max(this->_dependency_level,
                                           1 + j->calculate_dependency_level());
    }
    for (AtlasJob *j: this->_unknown_dependencies) {
        this->_dependency_level = std::max(this->_dependency_level,
                                           1 + j->calculate_dependency_level());
    }
    this->_dependency_level_calculated = true;
    return this->_dependency_level;
}

unsigned AtlasJob::estimated_execution_time_left(int timestamp) const {
    unsigned time_executed = this->time_executed(timestamp);
    if (time_executed > this->_execution_time_estimate) {
        return 0;
    }
    return this->_execution_time_estimate - time_executed;
}

bool AtlasJob::all_known_dependencies_finished(int timestamp) const {
    for (const AtlasJob *j: this->_known_dependencies) {
        if (j->finished(timestamp)) {
            continue;
        }
        return false;
    }
    return true;
}

bool AtlasJob::all_dependencies_finished(int timestamp) const {
    if (not this->all_known_dependencies_finished(timestamp)) {
        return false;
    }
    for (const AtlasJob *j: this->_unknown_dependencies) {
        if (j->finished(timestamp)) {
            continue;
        }
        return false;
    }
    return true;
}

bool AtlasJob::depends_on(const AtlasJob *job) const {
    return std::find(this->_known_dependencies.begin(), this->_known_dependencies.end(), job)
           != this->_known_dependencies.end();
}

void AtlasJob::set_atlas_schedule(AtlasSchedule *schedule) {
    this->_atlas_schedule = schedule;
    this->_schedules.push_back(schedule);
}

unsigned AtlasJob::time_executed(int timestamp) const {
    if (not this->all_dependencies_finished(timestamp)) {
        return 0;
    }
    unsigned time_executed = 0;
    for (BaseAtlasSchedule *s: this->_schedules) {
        auto data = s->get_data_at_time(timestamp);
        if (not data._does_execute) {
            continue;
        }
        unsigned value = data._execution_time;
        if (data._begin <= timestamp) {
            value = std::min<unsigned>(value, timestamp - data._begin);
        }
        if (data._scheduler == AtlasSchedulerType::CFS) {
            value /= this->_atlas_model->_cfs_factor;
        }
        time_executed += value;
    }
    return time_executed;
}

std::string AtlasJob::to_string() const {
    std::stringstream ss;
    ss << "j " << this->_id << " " << this->_deadline << " " << this->_execution_time_estimate
       << " " << this->_execution_time << " " << this->_submission_time
       << " 0" << std::endl;
    for (BaseAtlasSchedule *schedule: this->_schedules) {
        ss << schedule->to_string();
    }
    return ss.str();
}
