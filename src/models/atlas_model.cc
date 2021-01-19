#include <models/atlas_model.h>

const Schedule *AtlasModel::active_schedule(int timestamp) const {
    std::vector<Schedule *> schedules = this->_schedules;
    std::sort(schedules.begin(), schedules.end(), [=](const Schedule *a, const Schedule *b) {
            ScheduleData data_a = a->get_data_at_time(timestamp);
            ScheduleData data_b = b->get_data_at_time(timestamp);
            return (data_a._begin < data_b._begin);
        });
    for (const Schedule *s: schedules) {
        if (s->exists_at_time(timestamp)) {
            return s;
        }
    }
    return nullptr;
}

const Schedule *AtlasModel::active_schedule_on_scheduler(SchedulerType scheduler, int timestamp) const {
    std::vector<Schedule *> schedules = this->_schedules;
    std::sort(schedules.begin(), schedules.end(), [=](const Schedule *a, const Schedule *b) {
            ScheduleData data_a = a->get_data_at_time(timestamp);
            ScheduleData data_b = b->get_data_at_time(timestamp);
            return (data_a._begin < data_b._begin);
        });
    for (const Schedule *s: schedules) {
        ScheduleData data = s->get_data_at_time(timestamp);
        if (s->exists_at_time(timestamp) and data._scheduler == scheduler) {
            return s;
        }
    }
    return nullptr;
}

AtlasSchedule *AtlasModel::next_atlas_schedule() const {
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (data._begin >= this->_timestamp && s->_job->execution_time_left(this->_timestamp) > 0) {
            return s;
        }
    }
    return nullptr;
}

void AtlasModel::tidy_up_queues() {
    this->tidy_up_queue(&this->_cfs_queue);
    this->tidy_up_queue(&this->_recovery_queue);
}

void AtlasModel::tidy_up_queue(std::list<Job *> *queue) {
    std::list<Job *> old_queue(*queue);
    queue->clear();
    for (Job *j: old_queue) {
        if (j->execution_time_left(this->_timestamp) > 0) {
            queue->push_back(j);
        }
    }
}

void AtlasModel::reset_for_simulation() {
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
