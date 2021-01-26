#include <models/atlas_model.h>

bool compare_schedules(const Schedule *a, const Schedule *b) {
    return a->last_data()._begin < b->last_data()._begin;
}

AtlasModel::AtlasModel() : _schedules(compare_schedules), _atlas_schedules(compare_schedules) {}

void AtlasModel::add_atlas_schedule(AtlasSchedule *schedule) {
    this->_atlas_schedules.insert(schedule);
    this->_schedules.insert(schedule);
}

void AtlasModel::add_message(int timestamp, std::string text) {
    Message *message = new Message(timestamp, text);
    this->_messages.push_back(message);
    std::cerr << timestamp << ": " << text << std::endl;
}

const Schedule *AtlasModel::active_schedule(int timestamp) const {
    for (const Schedule *s: this->_schedules) {
        if (s->is_active_at_time(timestamp)) {
            return s;
        }
    }
    return nullptr;
}

const Schedule *AtlasModel::active_schedule_on_scheduler(SchedulerType scheduler, int timestamp) const {
    for (const Schedule *s: this->_schedules) {
        ScheduleData data = s->get_data_at_time(timestamp);
        if (s->is_active_at_time(timestamp) and data._scheduler == scheduler) {
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

std::vector<AtlasSchedule *> AtlasModel::next_atlas_schedules() const {
    std::vector<AtlasSchedule *> next_schedules;
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (data._begin >= this->_timestamp && s->_job->execution_time_left(this->_timestamp) > 0) {
            next_schedules.push_back(s);
        }
    }
    return next_schedules;
}

std::vector<Job *> AtlasModel::next_atlas_scheduled_jobs() const {
    std::vector<Job *> next_jobs;
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (data._begin >= this->_timestamp && s->_job->execution_time_left(this->_timestamp) > 0) {
            next_jobs.push_back(s->_job);
        }
    }
    return next_jobs;
}

void AtlasModel::tidy_up_queues() {
    this->tidy_up_queue(&this->_cfs_queue);
    this->tidy_up_queue(&this->_recovery_queue);
}

void AtlasModel::tidy_up_queue(std::list<Job *> *queue) {
    std::list<Job *> old_queue(*queue);
    queue->clear();
    for (Job *j: old_queue) {
        if (not j->finished(this->_timestamp)) {
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
