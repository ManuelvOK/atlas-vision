#include <cbs/cbs_simulation_model.h>


void CbsSimulationModel::add_job(HardRtJob *job) {
    this->_hard_rt_jobs.push_back(job);
    this->_jobs.push_back(job);
}

void CbsSimulationModel::add_job(SoftRtJob *job) {
    this->_soft_rt_jobs.push_back(job);
    this->_jobs.push_back(job);
}

void CbsSimulationModel::add_schedule(HardRtSchedule *schedule) {
    this->_hard_rt_schedules.push_back(schedule);
    this->_schedules.insert(schedule);
}

void CbsSimulationModel::enqueue_job(HardRtJob *job) {
    this->_hard_rt_queue.insert(job);
}

void CbsSimulationModel::dequeue_job(HardRtJob *job) {
    this->_hard_rt_queue.erase(job);
}


std::vector<BaseJob *> CbsSimulationModel::jobs() const {
    std::vector<BaseJob *> jobs(this->_hard_rt_jobs.begin(), this->_hard_rt_jobs.end());
    jobs.insert(jobs.end(), this->_soft_rt_jobs.begin(), this->_soft_rt_jobs.end());
    return jobs;
}

std::vector<BaseSchedule *> CbsSimulationModel::schedules() const {
    std::vector<BaseSchedule *> schedules(this->_hard_rt_schedules.begin(),
                                          this->_hard_rt_schedules.end());

    for (auto &[_, cbs]: this->_servers) {
        schedules.insert(schedules.end(), cbs._schedules.begin(), cbs._schedules.end());
    }

    return schedules;
}


CbsJob *CbsSimulationModel::next_job(unsigned skip) const {
    CbsJob *hard_rt_job = this->next_hard_rt_job(skip);
    CbsJob *soft_rt_job = this->next_soft_rt_job(skip);

    if (not soft_rt_job) {
        return hard_rt_job;
    }

    if (not hard_rt_job) {
        return soft_rt_job;
    }

    if (hard_rt_job->_deadline < soft_rt_job->_deadline) {
        return hard_rt_job;
    }
    return soft_rt_job;

}

HardRtJob *CbsSimulationModel::next_hard_rt_job(unsigned skip) const {
    if (skip >= this->_hard_rt_queue.size()) {
        return nullptr;
    }

    return *std::next(this->_hard_rt_queue.begin(), skip);
}

SoftRtJob *CbsSimulationModel::next_soft_rt_job(unsigned skip) const {
    std::set<SoftRtJob *, decltype(&compare_jobs_deadline<SoftRtJob>)>
        jobs(compare_jobs_deadline<SoftRtJob>);

    for (auto &[_, cbs]: this->_servers) {
        SoftRtJob *job = cbs.job();
        if (job) {
            jobs.insert(job);
        }
    }
    if (skip >= jobs.size()) {
        return nullptr;
    }

    return *std::next(jobs.begin(), skip);
}

CbsSchedule *CbsSimulationModel::active_schedule(unsigned core) const {
    CbsSchedule *schedule = this->_active_schedules.at(core);
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

