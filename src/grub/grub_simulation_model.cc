#include <grub/grub_simulation_model.h>

void GrubSimulationModel::add_job(HardGrubJob *job) {
    this->_hard_jobs.push_back(job);
    this->_jobs.push_back(job);
}

void GrubSimulationModel::add_job(SoftGrubJob *job) {
    this->_soft_jobs.push_back(job);
    this->_jobs.push_back(job);
}

void GrubSimulationModel::add_schedule(HardGrubSchedule *schedule) {
    this->_hard_schedules.push_back(schedule);
    this->_schedules.insert(schedule);
}

void GrubSimulationModel::enqueue_job(HardGrubJob *job) {
    this->_hard_queue.insert(job);
}

void GrubSimulationModel::dequeue_job(HardGrubJob *job) {
    this->_hard_queue.erase(job);
}

float GrubSimulationModel::total_utilisation() const {
    return this->_total_utilisation;
}

void GrubSimulationModel::change_total_utilisation(float difference) {
    this->update_servers_virtual_times();
    this->_total_utilisation += difference;
}

std::vector<BaseJob *> GrubSimulationModel::jobs() const {
    std::vector<BaseJob *> jobs(this->_hard_jobs.begin(), this->_hard_jobs.end());
    jobs.insert(jobs.end(), this->_soft_jobs.begin(), this->_soft_jobs.end());
    return jobs;
}

std::vector<BaseSchedule *> GrubSimulationModel::schedules() const {
    std::vector<BaseSchedule *> schedules(this->_hard_schedules.begin(),
                                          this->_hard_schedules.end());

    for (auto &[_, server]: this->_servers) {
        schedules.insert(schedules.end(), server._schedules.begin(),
                         server._schedules.end());
    }

    return schedules;
}

GrubJob *GrubSimulationModel::next_job(unsigned skip) const {
    GrubJob *hard_job = this->next_hard_job(skip);
    GrubJob *soft_job = this->next_soft_job(skip);

    if (not soft_job) {
        return hard_job;
    }

    if (not hard_job) {
        return soft_job;
    }

    if (hard_job->_deadline < soft_job->_deadline) {
        return hard_job;
    }
    return soft_job;

}

HardGrubJob *GrubSimulationModel::next_hard_job(unsigned skip) const {
    if (skip >= this->_hard_queue.size()) {
        return nullptr;
    }

    return *std::next(this->_hard_queue.begin(), skip);
}

SoftGrubJob *GrubSimulationModel::next_soft_job(unsigned skip) const {
    std::set<SoftGrubJob *, decltype(&compare_jobs_deadline<SoftGrubJob>)>
        jobs(compare_jobs_deadline<SoftGrubJob>);

    for (auto &[_, cbs]: this->_servers) {
        SoftGrubJob *job = cbs.job();
        if (job) {
            jobs.insert(job);
        }
    }
    if (skip >= jobs.size()) {
        return nullptr;
    }

    return *std::next(jobs.begin(), skip);
}

void GrubSimulationModel::update_servers_virtual_times() {
    for (auto &[_, cbs]: this->_servers) {
        cbs.update_virtual_time(this->_timestamp, this->_total_utilisation);
    }
}

GrubSchedule *GrubSimulationModel::active_schedule(unsigned core) const {
    GrubSchedule *schedule = this->_active_schedules.at(core);
    if (schedule) {
        return schedule;
    }
    for (auto &[_, cbs]: this->_servers) {
        if (cbs._active_schedule && cbs._active_schedule->_core == core) {
            return cbs._active_schedule;
        }
    }
    return nullptr;
}
