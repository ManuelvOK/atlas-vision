#include <cbs/cbs_simulation_action.h>

template<>
void CbsSubmissionAction<HardRtJob>::execute() {
    this->_model->enqueue_job(this->_job);

    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, this->_time));
}

template<>
void CbsSubmissionAction<SoftRtJob>::execute() {
    // TODO: change deadline
    ConstantBandwidthServer *cbs = this->_job->_cbs;
    cbs->enqueue_job(this->_job);

    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, this->_time));
}

void CbsDeadlineAction::execute() {
    // TODO: generate message
}

void CbsFillBudgetAction::execute() {
    unsigned timestamp = this->_model->_timestamp;
    if (this->_cbs->budget(timestamp) == 0) {
        this->_cbs->refill_budget(timestamp);
    }
}

void CbsFillAction::execute() {
    unsigned timestamp = this->_model->_timestamp;


    /* check if a job just started execution */
    CbsSchedule *active_schedule = this->_model->active_schedule();
    if (active_schedule && active_schedule->last_data()._begin == timestamp) {
        return;
    }

    /* check if the next job is hard rt or soft rt */
    CbsJob *next_job = this->_model->next_job();
    HardRtJob *next_hard_rt_job = this->_model->next_hard_rt_job();
    SoftRtJob *next_soft_rt_job = this->_model->next_soft_rt_job();

    if (not next_job) {
        return;
    }

    if (next_job == next_hard_rt_job) {
        /* generate schedule */
        HardRtSchedule *schedule =
            new HardRtSchedule(next_hard_rt_job, next_hard_rt_job->_submission_time, this->_core,
                               timestamp, next_hard_rt_job->_execution_time);
        next_hard_rt_job->_schedules.push_back(schedule);
        this->_model->add_schedule(schedule);

        /* add begin action */
        this->_model->_actions_to_do.push_back(
            new CbsBeginScheduleAction{this->_model, next_hard_rt_job, schedule});
    } else if (next_job == next_soft_rt_job) {
        SoftRtSchedule *schedule =
            new SoftRtSchedule(next_soft_rt_job, next_soft_rt_job->_submission_time, this->_core,
                               timestamp, next_soft_rt_job->_execution_time);
        next_soft_rt_job->_schedules.push_back(schedule);
        next_soft_rt_job->_cbs->add_schedule(schedule);

        /* add begin action */
        this->_model->_actions_to_do.push_back(
            new CbsBeginScheduleAction{this->_model, next_soft_rt_job, schedule});
    }
}

template<typename T>
unsigned CbsBeginScheduleAction<T>::time() const {
    return this->_schedule->last_data()._begin;
}

template<>
void CbsBeginScheduleAction<HardRtSchedule>::execute() {
    unsigned timestamp = this->_model->_timestamp;

    /* end active schedule */
    CbsSchedule *active_schedule = this->_model->_active_schedule;
    if (active_schedule) {
        active_schedule->add_change_end(timestamp, timestamp);
        active_schedule->end_simulation(timestamp);
    }

    /* Add End Action */
    this->_model->_active_schedule = this->_schedule;
}

template<>
void CbsBeginScheduleAction<SoftRtSchedule>::execute() {
    unsigned timestamp = this->_model->_timestamp;

    /* end active schedule */
    CbsSchedule *active_schedule = this->_model->_active_schedule;
    if (active_schedule) {
        active_schedule->add_change_end(timestamp, timestamp);
        active_schedule->end_simulation(timestamp);
    }

    /* Add End Action */
    this->_model->_active_schedule = this->_schedule;
}

template<>
void CbsEndScheduleAction<HardRtSchedule>::execute() {
    unsigned timestamp = this->_model->_timestamp;

    HardRtJob *job = this->_schedule->_rt_job;

    if (not job->finished(timestamp)) {
        return;
    }

    /* dequeue job */
    this->_model->dequeue_job(job);
    /* Add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, timestamp));
}

template<>
void CbsEndScheduleAction<SoftRtSchedule>::execute() {
    unsigned timestamp = this->_model->_timestamp;

    SoftRtJob *job = this->_schedule->_rt_job;

    if (not job->finished(timestamp)) {
        return;
    }

    /* dequeue job */
    job->_cbs->dequeue_job(job);

    /* Add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, timestamp, this->_schedule->_core));
}
