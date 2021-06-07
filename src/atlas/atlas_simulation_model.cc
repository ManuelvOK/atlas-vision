#include <atlas/atlas_simulation_model.h>

AtlasSimulationModel::AtlasSimulationModel() : SimulationModel(), _atlas_schedules(compare_schedules) {}

void AtlasSimulationModel::add_atlas_schedule(AtlasSchedule *schedule) {
    this->_atlas_schedules.insert(schedule);
    this->_schedules.insert(schedule);
}

void AtlasSimulationModel::add_recovery_schedule(RecoverySchedule *schedule) {
    this->_recovery_schedules.push_back(schedule);
    this->_schedules.insert(schedule);
}

void AtlasSimulationModel::add_cfs_schedule(CfsSchedule *schedule) {
    this->_cfs_schedules.push_back(schedule);
    this->_schedules.insert(schedule);
}

void AtlasSimulationModel::add_early_cfs_schedule(EarlyCfsSchedule *schedule) {
    this->_early_cfs_schedules.push_back(schedule);
    this->add_cfs_schedule(schedule);
}

const BaseAtlasSchedule *AtlasSimulationModel::active_schedule_on_scheduler(unsigned core, AtlasSchedulerType scheduler, int timestamp) const {
    for (const BaseAtlasSchedule *s: this->_schedules) {
        AtlasScheduleData data = s->get_data_at_time(timestamp);
        if (s->is_active_at_time(timestamp) and data._scheduler == scheduler and s->_core == core) {
            return s;
        }
    }
    return nullptr;
}

AtlasSchedule *AtlasSimulationModel::next_atlas_schedule(unsigned core) const {
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (s->_core == core
            and data._begin >= this->_timestamp
            and data._execution_time > 0
            and s->job()->execution_time_left(this->_timestamp) > 0) {
            return s;
        }
    }
    return nullptr;
}

std::vector<AtlasSchedule *> AtlasSimulationModel::next_atlas_schedules(unsigned core) const {
    std::vector<AtlasSchedule *> next_schedules;
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (s->_core == core
            and data._begin >= this->_timestamp
            and s->job()->execution_time_left(this->_timestamp) > 0) {
            next_schedules.push_back(s);
        }
    }
    return next_schedules;
}

std::vector<AtlasJob *> AtlasSimulationModel::next_atlas_scheduled_jobs(unsigned core) const {
    std::vector<AtlasJob *> next_jobs;
    for (AtlasSchedule *s: this->_atlas_schedules) {
        AtlasScheduleData data = s->get_data_at_time(this->_timestamp);
        if (s->_core == core
            and data._begin >= this->_timestamp
            and s->job()->execution_time_left(this->_timestamp) > 0) {
            next_jobs.push_back(s->atlas_job());
        }
    }
    return next_jobs;
}

unsigned AtlasSimulationModel::space_on_atlas(unsigned begin, unsigned end) const {
    if (end <= begin) {
        return 0;
    }
    unsigned space = end - begin;
    for (AtlasSchedule *schedule: this->_atlas_schedules) {
        AtlasScheduleData data = schedule->last_data();
        /* schedule not in given interval */
        if (data.end() < begin or data._begin > end) {
            continue;
        }
        /* given interval completely in schedule */
        if (data._begin < begin and data.end() > end) {
            return 0;
        }

        unsigned time_in_interval = data._execution_time;
        /* schedule begins earlier */
        if (data._begin < begin) {
            time_in_interval = data.end() - begin;
        /* schedule ends later */
        } else if(data.end() > end) {
            time_in_interval = end - data._begin;
        }

        if (time_in_interval >= space) {
            return 0;
        }

        space -= time_in_interval;
    }
    return space;
}

void AtlasSimulationModel::tidy_up_queues() {
    for (auto &[key, value]: this->_cfs_queue) {
        this->tidy_up_queue(&value);
    }
    for (auto &[key, value]: this->_recovery_queue) {
        this->tidy_up_queue(&value);
    }
}

void AtlasSimulationModel::resort_schedules() {
    SimulationModel<BaseAtlasSchedule, AtlasJob>::resort_schedules();

    auto old_atlas_schedules = this->_atlas_schedules;
    this->_atlas_schedules.clear();
    for (AtlasSchedule *s: old_atlas_schedules) {
        this->_atlas_schedules.insert(s);
    }
}

void AtlasSimulationModel::tidy_up_queue(std::list<AtlasJob *> *queue) {
    std::list<AtlasJob *> old_queue(*queue);
    queue->clear();
    for (AtlasJob *j: old_queue) {
        if (not j->finished(this->_timestamp)) {
            queue->push_back(j);
        }
    }
}
