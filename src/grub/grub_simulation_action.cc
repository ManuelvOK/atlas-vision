#include <grub/grub_simulation_action.h>


template<>
void GrubSubmissionAction<HardGrubJob>::execute() {
    std::cout << this->_model->_timestamp << ": HardRtJob " << this->_job->_id
              << " submitted. Deadline: " << this->_job->_deadline << std::endl;
    this->_model->enqueue_job(this->_job);

    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new GrubFillAction(this->_model, this->_time));
}

template<>
void GrubSubmissionAction<SoftGrubJob>::execute() {
    int timestamp = this->_model->_timestamp;
    std::cout << timestamp << ": SoftRtJob " << this->_job->_id << " submitted. " << std::endl;
    GrubConstantBandwidthServer *server = this->_job->_cbs;

    server->enqueue_job(this->_job);

    std::stringstream message;
    switch (server->_state) {
        case GrubState::INACTIVE:
            server->set_virtual_time(timestamp, timestamp);
            server->set_deadline(timestamp, timestamp + server->period());
            this->_job->add_change_deadline(timestamp, timestamp + server->period());
            this->_model->change_total_utilisation(server->processor_share());
            server->_state = GrubState::ACTIVE_CONTENDING;
            message << "Submission on Server " << server->id() << " that is inactive. Changing state to activeContending. New deadline: " << timestamp + server->period() << std::endl;
            this->_model->add_message(timestamp, message.str(), {this->_job->_id});
            break;
        case GrubState::ACTIVE_CONTENDING:
            this->_job->add_change_deadline(timestamp, server->deadline());
            message << "Submission on Server " << server->id() << " that already is activeContending. Deadline: " << server->deadline() << std::endl;
            this->_model->add_message(timestamp, message.str(), {this->_job->_id});
            break;
        case GrubState::ACTIVE_NON_CONTENDING:
            server->set_deadline(timestamp, timestamp + server->period());
            this->_job->add_change_deadline(timestamp, timestamp + server->period());
            server->_state = GrubState::ACTIVE_CONTENDING;
            message << "Submission on Server " << server->id() << " that is activeNonContending. Changing state to activeContending. New deadline: " << timestamp + server->period() << std::endl;
            this->_model->add_message(timestamp, message.str(), {this->_job->_id});
            break;
        default:
            break;
    }


    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new GrubFillAction(this->_model, this->_time));
}

void GrubFillAction::execute() {
    int timestamp = this->_model->_timestamp;

    for (unsigned core = 0; core < this->_model->_n_cores; ++core) {
        /* check if a job just started execution */
        GrubSchedule *active_schedule = this->_model->active_schedule(core);
        if (active_schedule && active_schedule->last_data()._begin == timestamp) {
            return;
        }

        /* end active schedule */
        if (active_schedule) {
            active_schedule->add_change_end(timestamp, timestamp);
            active_schedule->end_simulation(timestamp);
        }


        /* check if the next job is hard rt or soft rt */
        GrubJob *next_job = this->_model->next_job(core);
        HardGrubJob *next_hard_job = this->_model->next_hard_job(core);
        SoftGrubJob *next_soft_job = this->_model->next_soft_job(core);

        if (not next_job) {
            std::cout << timestamp << ": no next job found" << std::endl;
            return;
        }
        std::cout << timestamp << ": Start next job on core " << core << std::endl;

        if (next_job == next_hard_job) {
            std::stringstream message;
            message << "Next EDF: Job " << next_hard_job->_id << " (hard rt)"
                    << " Execution time left: " << next_hard_job->execution_time_left(timestamp)
                    << " Deadline: " << next_hard_job->_deadline;
            this->_model->add_message(timestamp, message.str(), {next_hard_job->_id});
            /* generate schedule */
            HardGrubSchedule *schedule =
                new HardGrubSchedule(next_hard_job, next_hard_job->_submission_time, core,
                                     timestamp, next_hard_job->execution_time_left(timestamp));
            next_hard_job->_schedules.push_back(schedule);
            this->_model->add_schedule(schedule);

            /* add begin action */
            this->_model->_actions_to_do.push_back(
                new GrubBeginScheduleAction{this->_model, next_hard_job, schedule});
        } else if (next_job == next_soft_job) {
            std::stringstream message;
            message << "Next EDF: Job " << next_soft_job->_id << " (soft rt, cbs "
                    << next_soft_job->_cbs->id() << ")" << " Execution time left: "
                    << next_soft_job->execution_time_left(timestamp) << " Deadline: "
                    << next_soft_job->deadline(timestamp);
            this->_model->add_message(timestamp, message.str(), {next_soft_job->_id});
            SoftGrubSchedule *schedule =
                new SoftGrubSchedule(next_soft_job, next_soft_job->_submission_time, core,
                                     timestamp, next_soft_job->execution_time_left(timestamp));
            next_soft_job->_schedules.push_back(schedule);
            next_soft_job->_cbs->add_schedule(schedule);

            /* add begin action */
            this->_model->_actions_to_do.push_back(
                new GrubBeginScheduleAction{this->_model, next_soft_job, schedule});
        }
    }
}

template<typename T>
int GrubBeginScheduleAction<T>::time() const {
    return this->_schedule->last_data()._begin;
}

template<>
void GrubBeginScheduleAction<HardGrubSchedule>::execute() {
    int timestamp = this->_model->_timestamp;

    this->_schedule->add_change_begin(timestamp, timestamp);
    this->_schedule->add_change_does_execute(timestamp, true);

    /* set this schedule as active */
    this->_model->_active_schedules[this->_schedule->_core] = this->_schedule;

    /* Add End Action */
    this->_model->_actions_to_do.push_back(
        new GrubEndScheduleAction(this->_model, this->_job, this->_schedule));
}

template<>
void GrubBeginScheduleAction<SoftGrubSchedule>::execute() {
    int timestamp = this->_model->_timestamp;

    this->_schedule->add_change_begin(timestamp, timestamp);
    this->_schedule->add_change_does_execute(timestamp, true);

    /* set this schedule as active */
    SoftGrubJob *job = this->_schedule->_grub_job;
    job->_cbs->_active_schedule = this->_schedule;
    job->_cbs->update_virtual_time(timestamp, this->_model->total_utilisation());
    job->_cbs->set_running(timestamp, true);

    /* Add End Action */
    this->_model->_actions_to_do.push_back(
        new GrubEndScheduleAction(this->_model, job, this->_schedule));

    std::stringstream message;
    message << "Starting schedule for job " << job->_id << " on Server " << job->_cbs->id() << std::endl
            << "    Total_utilisation: " << this->_model->total_utilisation() << std::endl
            << "    vruntime_inc: " << this->_model->total_utilisation() / job->_cbs->processor_share() << std::endl
            << "    Deadline postponement: " << job->_cbs->next_virtual_time_deadline_miss(this->_model->total_utilisation()) << std:: endl;
    this->_model->add_message(timestamp, message.str(), {job->_id});

    /* Add RefillBudget Action */
    this->_model->_actions_to_do.push_back(
        new GrubPostponeDeadlineAction(this->_model, job->_cbs));
}

int GrubPostponeDeadlineAction::time() const {
    return this->_server->next_virtual_time_deadline_miss(this->_model->total_utilisation());
}

void GrubPostponeDeadlineAction::execute() {
    int timestamp = this->_model->_timestamp;
    GrubConstantBandwidthServer *server = this->_server;
    if (not server->running()) {
        std::cout << timestamp << ": possible postponement of server " << server->id() << ". But is not running" << std::endl;
        return;
    }
    server->update_virtual_time(timestamp, this->_model->total_utilisation());
    int virtual_time = server->virtual_time();
    if (virtual_time < server->deadline()) {
        std::cout << timestamp << ": possible postponement of server " << server->id() << ". But seems like already happened." << std::endl
                  << "\t vtime: " << virtual_time << std::endl
                  << "\t deadline: " << server->deadline() << std::endl;

        return;
    }
    int new_deadline = server->deadline() + server->period();
    server->set_deadline(timestamp, new_deadline);
    for (SoftGrubJob *job: server->job_queue()) {
        job->add_change_deadline(timestamp, new_deadline);
    }

    std::stringstream message;
    message << "Virtual runtime of server " << server->id() << " equals deadline. New Deadline: " << new_deadline << std::endl;
    this->_model->add_message(timestamp, message.str());

    SoftGrubSchedule *active_schedule = server->_active_schedule;
    if (active_schedule) {
        active_schedule->add_change_end(timestamp, timestamp);
        active_schedule->end_simulation(timestamp);
    }

    /* add Fill Action */
    this->_model->_actions_to_do.push_back(
        new GrubFillAction(this->_model, timestamp));
}

template<typename T>
int GrubEndScheduleAction<T>::time() const {
    return this->_schedule->last_data().end();
}

template<>
void GrubEndScheduleAction<HardGrubSchedule>::execute() {
    int timestamp = this->_model->_timestamp;

    HardGrubJob *job = this->_schedule->_grub_job;
    this->_model->_active_schedules[this->_schedule->_core] = nullptr;

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
        new GrubFillAction(this->_model, timestamp));
}

template<>
void GrubEndScheduleAction<SoftGrubSchedule>::execute() {
    int timestamp = this->_model->_timestamp;

    SoftGrubJob *job = this->_schedule->_grub_job;
    GrubConstantBandwidthServer *server = job->_cbs;
    server->_active_schedule = nullptr;
    server->update_virtual_time(timestamp, this->_model->total_utilisation());
    server->set_running(timestamp, false);

    if (not job->finished(timestamp)) {
        std::cout << timestamp << ": execution for job " << this->_schedule->job()->_id
                  << " (soft) stopped but not finished." << std::endl
                  << "\tVirtual time: " << server->virtual_time() << std::endl;
        return;
    }

    std::cout << timestamp << ": execution for job " << this->_schedule->job()->_id
              << " (soft) finished." << std::endl
                << "\tVirtual time: " << server->virtual_time() << std::endl;

    /* dequeue job */
    job->_cbs->dequeue_job(job);

    /* change State if no next job */
    if (server->job_queue().empty()) {
        std::cout << timestamp << ": No next job. Going ActiveNonContending, Inactive at " << server->virtual_time() << std::endl;
        server->_state = GrubState::ACTIVE_NON_CONTENDING;
        /* Add DeactivateAction */
        this->_model->_actions_to_do.push_back(
            new GrubDeactivateServerAction(this->_model, server->virtual_time(), server));
    }

    /* Add Fill Action */
    this->_model->_actions_to_do.push_back(
        new GrubFillAction(this->_model, timestamp));
}

int GrubDeactivateServerAction::time() const {
    return this->_server->virtual_time();
}

void GrubDeactivateServerAction::execute() {
    int timestamp = this->_model->_timestamp;
    GrubConstantBandwidthServer *server = this->_server;

    /* TODO: Why should this ever happen? */
    int virtual_time = server->virtual_time();
    if (virtual_time > timestamp) {
        std::cout << timestamp << ": tried to deactivate server " << server->id() << " but virtual time " << virtual_time << " > now " << std::endl;
        return;
    }

    /* already is inactive */
    if (server->_state != GrubState::ACTIVE_NON_CONTENDING) {
        std::cout << timestamp << ": tried to deactivate server " << server->id() << " but not in ActiveNonContending." << std::endl;
        return;
    }

    /* change server's state */
    server->_state = GrubState::INACTIVE;

    /* alter total_utilisation */
    this->_model->change_total_utilisation(-server->processor_share());

    std::stringstream message;
    message << "Server " << server->id() << " goes inactive." << std::endl;
    for (auto &[_, other_server]: this->_model->_servers) {
        if (other_server.id() == server->id()) {
            continue;
        }
        message << "    Vtime Server " << other_server.id() << ": " << other_server.virtual_time() << std::endl
                << "    Deadline postponement Server: " << other_server.id() << ": " << other_server.next_virtual_time_deadline_miss(this->_model->total_utilisation()) << std:: endl;
    }
    this->_model->add_message(timestamp, message.str());

}
