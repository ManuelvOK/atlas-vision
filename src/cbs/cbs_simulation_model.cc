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


CbsJob *CbsSimulationModel::next_job() const {
    CbsJob *hard_rt_job = this->next_hard_rt_job();
    CbsJob *soft_rt_job = this->next_soft_rt_job();

    if (not soft_rt_job) {
        return hard_rt_job;
    }

    if (not hard_rt_job) {
        return soft_rt_job;
    }

    if (compare_jobs_deadline(hard_rt_job, soft_rt_job)) {
        return hard_rt_job;
    }
    return soft_rt_job;

}

HardRtJob *CbsSimulationModel::next_hard_rt_job() const {
    return *this->_hard_rt_queue.begin();
}

SoftRtJob *CbsSimulationModel::next_soft_rt_job() const {
    std::set<SoftRtJob *, decltype(&compare_jobs_deadline<SoftRtJob>)>
    jobs(compare_jobs_deadline<SoftRtJob>);

    for (auto &[_, cbs]: this->_servers) {
        SoftRtJob *job = cbs.job();
        if (job) {
            jobs.insert(job);
        }
    }
    return *jobs.begin();
}


CbsSchedule *CbsSimulationModel::active_schedule() const {
    if (this->_active_schedule) {
        return this->_active_schedule;
    }
    for (auto &[_, cbs]: this->_servers) {
        if (cbs._active_schedule) {
            return cbs._active_schedule;
        }
    }
    return nullptr;
}

