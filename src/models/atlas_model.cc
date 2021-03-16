#include <models/atlas_model.h>

bool compare_schedules(const Schedule *a, const Schedule *b) {
    int begin_a = a->last_data()._begin;
    int begin_b = b->last_data()._begin;
    if (begin_a == begin_b) {
        return a->_id < b->_id;
    }
    return begin_a < begin_b;
}

AtlasModel::AtlasModel() : _schedules(compare_schedules), _atlas_schedules(compare_schedules) {}

void AtlasModel::add_atlas_schedule(AtlasSchedule *schedule) {
    this->_atlas_schedules.insert(schedule);
    this->_schedules.insert(schedule);
}

void AtlasModel::add_recovery_schedule(RecoverySchedule *schedule) {
    this->_recovery_schedules.push_back(schedule);
    this->_schedules.insert(schedule);
}

void AtlasModel::add_cfs_schedule(CfsSchedule *schedule) {
    this->_cfs_schedules.push_back(schedule);
    this->_schedules.insert(schedule);
}

void AtlasModel::add_early_cfs_schedule(EarlyCfsSchedule *schedule) {
    this->_early_cfs_schedules.push_back(schedule);
    this->add_cfs_schedule(schedule);
}

void AtlasModel::add_message(int timestamp, std::string text) {
    Message *message = new Message(timestamp, text);
    this->_messages.push_back(message);
    std::cerr << timestamp << ": " << text << std::endl;
}

const Schedule *AtlasModel::active_schedule(unsigned core, int timestamp) const {
    for (const Schedule *s: this->_schedules) {
        if (s->is_active_at_time(timestamp) and s->_core == core) {
            return s;
        }
    }
    return nullptr;
}

const Schedule *AtlasModel::active_schedule_on_scheduler(unsigned core, SchedulerType scheduler, int timestamp) const {
    for (const Schedule *s: this->_schedules) {
        ScheduleData data = s->get_data_at_time(timestamp);
        if (s->is_active_at_time(timestamp) and data._scheduler == scheduler and s->_core == core) {
            return s;
        }
    }
    return nullptr;
}

AtlasSchedule *AtlasModel::next_atlas_schedule(unsigned core) const {
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (s->_core == core
            and data._begin >= this->_timestamp
            and s->_job->execution_time_left(this->_timestamp) > 0) {
            return s;
        }
    }
    return nullptr;
}

std::vector<AtlasSchedule *> AtlasModel::next_atlas_schedules(unsigned core) const {
    std::vector<AtlasSchedule *> next_schedules;
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (s->_core == core
            and data._begin >= this->_timestamp
            and s->_job->execution_time_left(this->_timestamp) > 0) {
            next_schedules.push_back(s);
        }
    }
    return next_schedules;
}

std::vector<Job *> AtlasModel::next_atlas_scheduled_jobs(unsigned core) const {
    std::vector<Job *> next_jobs;
    for (AtlasSchedule *s: this->_atlas_schedules) {
        ScheduleData data = s->get_data_at_time(this->_timestamp);
        if (s->_core == core
            and data._begin >= this->_timestamp
            and s->_job->execution_time_left(this->_timestamp) > 0) {
            next_jobs.push_back(s->_job);
        }
    }
    return next_jobs;
}

void AtlasModel::tidy_up_queues() {
    for (auto &[key, value]: this->_cfs_queue) {
        this->tidy_up_queue(&value);
    }
    for (auto &[key, value]: this->_recovery_queue) {
        this->tidy_up_queue(&value);
    }
}

void AtlasModel::resort_schedules() {
    auto old_schedules = this->_schedules;
    this->_schedules.clear();
    for (Schedule *s: old_schedules) {
        this->_schedules.insert(s);
    }

    auto old_atlas_schedules = this->_atlas_schedules;
    this->_atlas_schedules.clear();
    for (AtlasSchedule *s: old_atlas_schedules) {
        this->_atlas_schedules.insert(s);
    }
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
