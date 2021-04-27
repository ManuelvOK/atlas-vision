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

int AtlasJob::calculate_dependency_level() {
    /* only calculate if already set */
    if (this->_dependency_level >= 0) {
        return this->_dependency_level;
    }
    if (this->_known_dependencies.size() == 0 && this->_unknown_dependencies.size() == 0) {
        this->_dependency_level = 0;
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
    return this->_dependency_level;
}

int AtlasJob::estimated_execution_time_left(int timestamp) const {
    return this->_execution_time_estimate - this->time_executed(timestamp);
}

bool AtlasJob::all_dependencies_finished(int timestamp) const {
    for (const AtlasJob *j: this->_known_dependencies) {
        if (j->finished(timestamp)) {
            continue;
        }
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

int AtlasJob::time_executed(int timestamp) const {
    if (not this->all_dependencies_finished(timestamp)) {
        return 0;
    }
    int time_executed = 0;
    for (BaseAtlasSchedule *s: this->_schedules) {
        auto data = s->get_data_at_time(timestamp);
        if (not data._does_execute) {
            continue;
        }
        int value = std::min(data._execution_time, std::max(0, timestamp - data._begin));
        if (data._scheduler == AtlasSchedulerType::CFS) {
            value /= this->_atlas_model->_cfs_factor;
        }
        time_executed += value;
    }
    return time_executed;
}
