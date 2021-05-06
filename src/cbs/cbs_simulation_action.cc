#include <cbs/cbs_simulation_action.h>

#include <iostream>

template<>
void CbsSubmissionAction<HardRtJob>::execute() {
    std::cout << this->_model->_timestamp << ": HardRtJob " << this->_job->_id
              << " submitted. Deadline: " << this->_job->_deadline << std::endl;
    this->_model->enqueue_job(this->_job);

    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, this->_time));
}

template<>
void CbsSubmissionAction<SoftRtJob>::execute() {
    unsigned timestamp = this->_model->_timestamp;
    std::cout << timestamp << ": SoftRtJob " << this->_job->_id << " submitted. " << std::endl;
    ConstantBandwidthServer *cbs = this->_job->_cbs;


    /* change deadline */
    unsigned deadline = cbs->deadline(timestamp);
    if (not cbs->is_active()) {
        std::cout << timestamp << ": cbs " << cbs->id() << " is inactive" << std::endl;
        unsigned current_budget = cbs->budget(timestamp);
        std::cout << timestamp << ":\tc_s: " << current_budget << "\td_s_k: " << deadline
                  << "\tr_i_j: " << timestamp << "\tU_s: " << cbs->utilisation() << std::endl;
        std::cout << timestamp << ":\t(d_s_k - r_i_j) * U_s: "
                  << (static_cast<int>(deadline) - static_cast<int>(timestamp)) * cbs->utilisation() << std::endl;
        if (current_budget >= (static_cast<int>(deadline) - static_cast<int>(timestamp)) * cbs->utilisation()) {
            deadline = cbs->generate_new_deadline_and_refill(timestamp);

            std::stringstream message;
            message << "Generated new deadline for cbs " << cbs->id() << ": " << deadline;
            this->_model->add_message(timestamp, message.str());
        }
    }
    std::stringstream message;
    message << "Schedule for job" << this->_job->_id << " gets new dealine: " << deadline << std::endl;
    this->_model->add_message(timestamp, message.str());
    this->_job->add_change_deadline(timestamp, deadline);

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
    if (this->_cbs->budget(timestamp) != 0) {
        std::cout << timestamp << ": try to refill budget for cbs " << this->_cbs->id() << " but its not 0." << std::endl;
        return;
    }
    unsigned deadline = this->_cbs->generate_new_deadline_and_refill(timestamp);

    std::stringstream message;
    message << "Refill Budget for cbs " << this->_cbs->id() << ". New deadline: " << deadline;
    this->_model->add_message(timestamp, message.str());

    for (SoftRtJob *job: this->_cbs->job_queue()) {
        if (job->finished(timestamp)) {
            continue;
        }
        std::stringstream message;
        message << "Job " << job->_id << " gets new deadline: " << deadline;
        this->_model->add_message(timestamp, message.str());

        job->add_change_deadline(timestamp, deadline);
    }
    SoftRtSchedule *active_schedule = this->_cbs->_active_schedule;
    if (active_schedule) {
        active_schedule->add_change_end(timestamp, timestamp);
        active_schedule->end_simulation(timestamp);
    }

    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, this->_time));
}

void CbsFillAction::execute() {
    unsigned timestamp = this->_model->_timestamp;


    /* check if a job just started execution */
    CbsSchedule *active_schedule = this->_model->active_schedule();
    if (active_schedule && active_schedule->last_data()._begin == timestamp) {
        return;
    }

    /* end active schedule */
    if (active_schedule) {
        active_schedule->add_change_end(timestamp, timestamp);
        active_schedule->end_simulation(timestamp);
    }


    /* check if the next job is hard rt or soft rt */
    CbsJob *next_job = this->_model->next_job();
    HardRtJob *next_hard_rt_job = this->_model->next_hard_rt_job();
    SoftRtJob *next_soft_rt_job = this->_model->next_soft_rt_job();

    if (not next_job) {
        return;
    }
    std::cout << timestamp << ": Start next job" << std::endl;

    if (next_job == next_hard_rt_job) {
        std::stringstream message;
        message << "Next EDF: Job " << next_hard_rt_job->_id << " (hard rt)"
                << " Execution time left: " << next_hard_rt_job->execution_time_left(timestamp)
                << " Deadline: " << next_hard_rt_job->_deadline;
        this->_model->add_message(timestamp, message.str());
        /* generate schedule */
        HardRtSchedule *schedule =
            new HardRtSchedule(next_hard_rt_job, next_hard_rt_job->_submission_time, this->_core,
                               timestamp, next_hard_rt_job->execution_time_left(timestamp));
        next_hard_rt_job->_schedules.push_back(schedule);
        this->_model->add_schedule(schedule);

        /* add begin action */
        this->_model->_actions_to_do.push_back(
            new CbsBeginScheduleAction{this->_model, next_hard_rt_job, schedule});
    } else if (next_job == next_soft_rt_job) {
        std::stringstream message;
        message << "Next EDF: Job " << next_soft_rt_job->_id << " (soft rt, cbs "
                << next_soft_rt_job->_cbs->id() << ")" << " Execution time left: "
                << next_soft_rt_job->execution_time_left(timestamp) << " Deadline: "
                << next_soft_rt_job->deadline(timestamp);
        this->_model->add_message(timestamp, message.str());
        SoftRtSchedule *schedule =
            new SoftRtSchedule(next_soft_rt_job, next_soft_rt_job->_submission_time, this->_core,
                               timestamp, next_soft_rt_job->execution_time_left(timestamp));
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

    this->_schedule->add_change_begin(timestamp, timestamp);
    this->_schedule->add_change_does_execute(timestamp, true);

    /* set this schedule as active */
    this->_model->_active_schedule = this->_schedule;

    /* Add End Action */
    this->_model->_actions_to_do.push_back(
        new CbsEndScheduleAction(this->_model, this->_job, this->_schedule));
}

template<>
void CbsBeginScheduleAction<SoftRtSchedule>::execute() {
    unsigned timestamp = this->_model->_timestamp;

    this->_schedule->add_change_begin(timestamp, timestamp);
    this->_schedule->add_change_does_execute(timestamp, true);

    /* set this schedule as active */
    SoftRtJob *job = this->_schedule->_rt_job;
    job->_cbs->_active_schedule = this->_schedule;

    /* Add End Action */
    this->_model->_actions_to_do.push_back(
        new CbsEndScheduleAction(this->_model, job, this->_schedule));

    /* Add RefillBudget Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillBudgetAction(this->_model, timestamp + job->_cbs->budget(timestamp), job->_cbs));

    std::cout << timestamp << ": Cbs " << job->_cbs->id()
              << " has budget of " << job->_cbs->budget(timestamp)
              << ". Adding FillBudgetAction for " << timestamp + job->_cbs->budget(timestamp)
              << std::endl;
}

template<typename T>
unsigned CbsEndScheduleAction<T>::time() const {
    return this->_schedule->last_data().end();
}

template<>
void CbsEndScheduleAction<HardRtSchedule>::execute() {
    unsigned timestamp = this->_model->_timestamp;

    HardRtJob *job = this->_schedule->_rt_job;
    this->_model->_active_schedule = nullptr;

    if (not job->finished(timestamp)) {
        std::cout << timestamp << ": execution for job " << this->_schedule->job()->_id
                  << " (hard) stopped but not finished." << std::endl;
        return;
    }
    std::cout << timestamp << ": execution for job " << this->_schedule->job()->_id
              << " (hard) finished." << std::endl;

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
    job->_cbs->_active_schedule = nullptr;

    if (not job->finished(timestamp)) {
        std::cout << timestamp << ": execution for job " << this->_schedule->job()->_id
                  << " (soft) stopped but not finished." << std::endl;
        return;
    }

    std::cout << timestamp << ": execution for job " << this->_schedule->job()->_id
              << " (soft) finished." << std::endl;

    /* dequeue job */
    job->_cbs->dequeue_job(job);

    /* Add Fill Action */
    this->_model->_actions_to_do.push_back(
        new CbsFillAction(this->_model, timestamp, this->_schedule->_core));
}
