#include <atlas/atlas_simulation_action.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

#include <atlas/atlas_simulation_model.h>

void AtlasSubmissionAction::execute() {
    int timestamp = this->_model->_timestamp;
    unsigned core = this->_core_assigner->get_core_for_job(this->_job);
    /* add scheduled jobs to the list of jobs to schedule */
    std::vector<AtlasJob *> jobs = this->_model->next_atlas_scheduled_jobs(core);
    jobs.push_back(this->_job);
    /* sort jobs EDF wise while considering dependencies */
    std::sort(jobs.begin(), jobs.end(), [=](const AtlasJob *a, const AtlasJob *b) {
        if (b->depends_on(a)) {
            return true;
        }
        if (a->depends_on(b)) {
            return false;
        }
        /* independent */
        if (a->_deadline != b->_deadline) {
            return a->_deadline < b->_deadline;
        }
        /* keep order on jobs with same dealine */
        return a->_id < b->_id;
    });

    /* iterate through the EDF sorted jobs in reversed order to add schedules while minimizing
     * slack. */
    int max_end = std::numeric_limits<int>::max();
    for (auto job_it = jobs.rbegin(); job_it != jobs.rend(); ++job_it) {
        AtlasJob *job = *job_it;
        max_end = std::min(max_end, job->_deadline);

        /* if there already is a schedule, we might have to adjust it */
        if (job->_atlas_schedule != nullptr) {
            AtlasScheduleData data = job->_atlas_schedule->last_data();
            int shift_value = max_end - data.end();
            if (shift_value < 0) {
                job->_atlas_schedule->add_change_shift_relative(timestamp,
                                                                shift_value);
                std::cout << "Schedule for Job " << job->_id << " shifted by " << shift_value
                          << "." << std::endl;
            }
            max_end = std::min(max_end, job->_atlas_schedule->last_data()._begin);
            this->_model->resort_schedules();
            continue;
        }

        /* create a new schedule */
        int begin = max_end - job->_execution_time_estimate;
        AtlasSchedule *s = new AtlasSchedule (job, timestamp, core, begin,
                                              job->_execution_time_estimate);
        job->set_atlas_schedule(s);
        this->_model->add_atlas_schedule(s);

        /* queue an action to begin the schedule. */
        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, job, s});

        /* queue an action for the deadline of the job */
        this->_model->_actions_to_do.push_back(
            new AtlasDeadlineAction(this->_model, job));

        std::stringstream message;
        message << "Job " << job->_id << " submitted and scheduled on core " << core
                << " ATLAS from " << begin << " for " << job->_execution_time_estimate << ".";
        this->_model->add_message(timestamp, message.str(), {job->_id});
        max_end = std::min(max_end, begin);

    }


    /* check if there is an active schedule on CFS and if not, queue action to fill CFS */
    if (not this->_model->active_schedule_on_scheduler(core, AtlasSchedulerType::CFS,
                                                             this->_model->_timestamp)
        and not this->_model->active_schedule_on_scheduler(core, AtlasSchedulerType::ATLAS,
                                                                 this->_model->_timestamp)) {
        std::cerr << this->_model->_timestamp << ": CFS on core " << core
                  << " is free and has to be filled." << std::endl;
        this->_model->_actions_to_do.push_back(
            new AtlasFillAction(this->_model, this->_model->_timestamp, core));
    }
}


void AtlasDeadlineAction::execute() {
    /* check if job has finished or if there is execution time left */
    if (this->_job->execution_time_left(this->_model->_timestamp) > 0) {
        std::cerr << this->_model->_timestamp << ": Deadline miss for job "
                  << this->_job->_id << std::endl;
    } else {
        std::cerr << this->_model->_timestamp << ": Deadline for job "
                  << this->_job->_id << std::endl;
    }
}

void AtlasFillAction::execute() {
    int timestamp = this->_model->_timestamp;
    /* check whether there is an active schedule anywhere. If so, do nothing */
    auto s = this->_model->active_schedule(this->_core, timestamp);
    if (s) {
        std::cerr << timestamp << ": tried to fill core " << this->_core << " but job "
                  << s->job()->_id << " was runing until "
                  << s->last_data()._begin + s->last_data()._execution_time << std::endl;
        return;
    }
    this->_model->tidy_up_queues();

    /* Check if there is something on recovery */
    if (not this->_model->_recovery_queue[this->_core].empty()) {
        /* Create Schedule on recovery and queue end action */
        AtlasJob *job = this->_model->_recovery_queue[this->_core].front();

        int time_left = job->estimated_execution_time_left(timestamp);
        BaseAtlasSchedule *next_atlas_schedule =
            this->_model->next_atlas_schedule(this->_core);
        int time_to_next_atlas = time_left;
        if (next_atlas_schedule) {
            time_to_next_atlas = next_atlas_schedule->last_data()._begin - timestamp;
        }
        int recovery_time = std::min(time_left, time_to_next_atlas);
        RecoverySchedule *recovery_schedule =
            new RecoverySchedule(job, timestamp, this->_core, timestamp, recovery_time);

        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, job, recovery_schedule});

    /* Check if there is something in CFS queue */
    } else if (not this->_model->_cfs_queue[this->_core].empty()) {
        AtlasJob *job = this->_model->_cfs_queue[this->_core].front();
        /* Create Schedule on cfs and queue end action */
        LateCfsSchedule *cfs_schedule =
            new LateCfsSchedule(job, timestamp, this->_core, timestamp,
                                job->execution_time_left(timestamp)
                                * this->_model->_cfs_factor);

        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, job, cfs_schedule});
    } else {
        /* find next schedule on ATLAS */
        AtlasSchedule *next_atlas_schedule = this->_model->next_atlas_schedule(this->_core);
        /* if there is none, abort */
        if (not next_atlas_schedule) {
            return;
        }
        AtlasJob *job = next_atlas_schedule->atlas_job();
        std::cerr << timestamp << ": next atlas schedule found on core " << this->_core
                  << " for job " << job->_id << std::endl;

        /* create schedule on CFS based on next atlas schedule and adjust it */
        int execution_time = job->execution_time_left(timestamp) * this->_model->_cfs_factor;
        execution_time = std::min(execution_time, next_atlas_schedule->last_data()._begin);
        EarlyCfsSchedule *cfs_schedule = new EarlyCfsSchedule(next_atlas_schedule, timestamp,
                                                              timestamp, execution_time);

        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, cfs_schedule->atlas_job(), cfs_schedule});
    }

}

template<typename T>
void AtlasBeginScheduleAction<T>::end_schedule(BaseAtlasSchedule *schedule) {
    if (not schedule) {
        return;
    }
    int timestamp = this->_model->_timestamp;
    std::cerr << timestamp << ": End '" << (char)schedule->last_data()._scheduler
              << "' schedule on core " << schedule->_core
              << " for job " << schedule->job()->_id << std::endl;
    schedule->add_change_end(timestamp, timestamp);
    schedule->end_simulation(timestamp);

    /* schedule no longer executes */
    if (this->_model->_cfs_schedule[schedule->_core] == schedule) {
        this->_model->_cfs_schedule[schedule->_core] = nullptr;
    }
    if (this->_model->_recovery_schedule[schedule->_core] == schedule) {
        this->_model->_recovery_schedule[schedule->_core] = nullptr;
    }
}

template<typename T>
void AtlasBeginScheduleAction<T>::add_end_action() const {
    this->_model->_actions_to_do.push_back(
        new AtlasEndScheduleAction{this->_model, this->_job, this->_schedule});
}

template<typename T>
int AtlasBeginScheduleAction<T>::time() const{
    return this->_schedule->last_data()._begin;
}

template<>
void AtlasBeginScheduleAction<LateCfsSchedule>::execute() {
    int timestamp = this->_model->_timestamp;
    this->_schedule->atlas_job()->_schedules.push_back(this->_schedule);
    this->_model->add_cfs_schedule(this->_schedule);

    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    int time_left = this->_schedule->job()->execution_time_left(timestamp);
    message << "Execute job " << this->_schedule->job()->_id << " on core "
            << this->_schedule->_core << " CFS (late). Time left: "
            << time_left << " (" << time_left * this->_model->_cfs_factor << " on CFS)";
    this->_model->add_message(timestamp, message.str(), {this->_schedule->job()->_id});

    this->_model->_cfs_schedule[this->_schedule->_core] = this->_schedule;
    /* add AtlasEndAction */
    this->add_end_action();
}

template<>
void AtlasBeginScheduleAction<EarlyCfsSchedule>::execute() {
    int timestamp = this->_model->_timestamp;

    /* check if schedule has been ended by other event */
    if (this->_schedule->_simulation_ended) {
        return;
    }

    /* check if we already have tried to add this schedule */
    if (this->_model->_cfs_schedule[this->_schedule->_core] != this->_schedule) {
        this->_model->_cfs_schedule[this->_schedule->_core] = this->_schedule;
        this->_schedule->atlas_job()->_schedules.push_back(this->_schedule);
        this->_model->add_early_cfs_schedule(this->_schedule);
    }

    /* check if dependencies have finished */
    if (not this->_schedule->atlas_job()->all_dependencies_finished(timestamp)) {
        std::cerr << timestamp << ": Can not start cfs schedule on core "
                  << this->_schedule->_core << " for job " << this->_schedule->job()->_id
                  << " because of dependencies. Waiting..." << std::endl;
        this->_success = false;
        return;
    }

    /* update begin time if necessary */
    if (this->_schedule->last_data()._begin != timestamp) {
        this->_schedule->add_change_begin(timestamp, timestamp, false);
    }

    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    int time_left = this->_schedule->job()->execution_time_left(timestamp);
    message << "Execute job " << this->_schedule->job()->_id << " on core "
            << this->_schedule->_core << " CFS (late). Time left: "
            << time_left << " (" << time_left * this->_model->_cfs_factor << " on CFS)";
    this->_model->add_message(timestamp, message.str(), {this->_schedule->job()->_id});

    /* add AtlasEndAction */
    this->add_end_action();
}

template<>
void AtlasBeginScheduleAction<RecoverySchedule>::execute() {
    int timestamp = this->_model->_timestamp;

    /* check if schedule has been ended by other event */
    if (this->_schedule->_simulation_ended) {
        return;
    }

    /* check if we already have tried to add this schedule */
    if (this->_model->_recovery_schedule[this->_schedule->_core] != this->_schedule) {
        this->_model->_recovery_schedule[this->_schedule->_core] = this->_schedule;
        this->_schedule->atlas_job()->_schedules.push_back(this->_schedule);
        this->_model->add_recovery_schedule(this->_schedule);
    }

    /* check if dependencies have finished */
    if (not this->_schedule->atlas_job()->all_dependencies_finished(timestamp)) {
        std::cerr << timestamp << ": Can not start recovery schedule on core "
                  << this->_schedule->_core << " for job " << this->_schedule->job()->_id
                  << " because of dependencies. Waiting..." << std::endl;
        this->_success = false;
        return;
    }

    /* update begin time if necessary */
    AtlasScheduleData data = this->_schedule->last_data();
    if (data._begin != timestamp) {
        this->_schedule->add_change_begin(timestamp, timestamp, false);
        int next_atlas_schedule_begin =
            this->_model->next_atlas_schedule(this->_schedule->_core)
            ->last_data()._begin;
        data = this->_schedule->last_data();
        if (data.end() > next_atlas_schedule_begin) {
            this->_schedule->add_change_end(timestamp, next_atlas_schedule_begin);
        }
    }

    /* stop cfs if running */
    this->end_schedule(this->_model->_cfs_schedule[this->_schedule->_core]);

    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    message << "Execute job " << this->_schedule->job()->_id << " on core " << this->_schedule->_core
            << " recovery. Scheduled time: " << this->_schedule->last_data()._execution_time;
    this->_model->add_message(timestamp, message.str(), {this->_schedule->job()->_id});

    this->_model->_recovery_schedule[this->_schedule->_core] = this->_schedule;
    /* add AtlasEndAction */
    this->add_end_action();
}

template<>
void AtlasBeginScheduleAction<AtlasSchedule>::execute() {
    int timestamp = this->_model->_timestamp;
    AtlasJob *job = this->_schedule->atlas_job();

    /* check if ATLAS schedule still exists */
    if (job->execution_time_left(timestamp) <= 0) {
        return;
    }

    /* check if there is a schedule already running */
    BaseAtlasSchedule *current_schedule = job->schedule_at_time(timestamp);
    if (current_schedule
        && current_schedule != this->_schedule
        && current_schedule->last_data()._scheduler == AtlasSchedulerType::ATLAS) {
        std::cerr << timestamp << ": job " << job->_id << " already runs somewhere else"
                  << std::endl;
        return;
    }

    /* update begin time if necessary */
    AtlasScheduleData data = this->_schedule->last_data();
    if (data._begin != timestamp) {
        this->_schedule->add_change_begin(timestamp, timestamp, false);
        this->_schedule->add_change_end(timestamp, data.end());
    }

    /* stop everything thats running */
    this->end_schedule(this->_model->_cfs_schedule[this->_schedule->_core]);
    this->end_schedule(this->_model->_recovery_schedule[this->_schedule->_core]);

    /* check dependencies */
    /* TODO: check for all dependencies, not only the first one. this was quick and dirty */
    AtlasJob *dependent_job = job;
    std::vector<AtlasJob *> dependencies = dependent_job->known_dependencies();
    int dependency_time_left = 0;
    /* aggregate time that all the dependent jobs in the dependency chain have left to run */
    while (dependencies.size() > 0) {
        int time_left = dependencies[0]->execution_time_left(timestamp);
        if (time_left <= 0) {
            break;
        }
        dependent_job = dependencies[0];
        dependencies = dependent_job->known_dependencies();
        dependency_time_left = time_left;
    }
    if (dependent_job != job) {
        BaseAtlasSchedule *dependent_schedule = dependent_job->schedule_at_time(timestamp);
        if (dependent_schedule
            && dependent_schedule->last_data()._scheduler == AtlasSchedulerType::ATLAS) {
            std::cerr << timestamp << ": job " << job->_id << " depends on job "
                      << dependent_job->_id << " that is already running. Waiting." << std::endl;
            this->_success = false;
            return;
        }
        std::stringstream message;
        message << timestamp << ": job " << job->_id << " depends on job " << dependent_job->_id
                << ". Inserting ATLAS dependency schedule" << std::endl;
        this->_model->add_message(timestamp, message.str(), {job->_id});
        int length = std::min(this->_schedule->last_data()._execution_time, dependency_time_left);

        DependencySchedule *schedule =
            new DependencySchedule(dependent_job, timestamp, this->_schedule->_core, timestamp,
                                   length, false);
        dependent_job->_schedules.push_back(schedule);
        this->_model->add_atlas_schedule(schedule);
        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, dependent_job, schedule});

        this->_schedule->add_change_does_execute(timestamp, false);

        if (this->_schedule->last_data()._execution_time > dependency_time_left) {
            /* dependency schedule comes to an end before schedule would finish */
            std::cerr << timestamp << ": there's time for job " << job->_id
                      << " after added schedule finished." << std::endl;
            this->_schedule->add_change_shift_relative(timestamp + dependency_time_left, length);
            this->_schedule->add_change_execution_time_relative(timestamp + dependency_time_left,
                                                                -dependency_time_left);
            this->_model->_actions_to_do.push_back(
                new AtlasBeginScheduleAction{this->_model, this->_job, this->_schedule});
        } else {
            this->_schedule->add_change_delete(timestamp);
        }
        return;
    }

    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    message << "Execute job " << job->_id << " on core " << this->_schedule->_core
            << " ATLAS. Time left: " << this->_schedule->last_data()._execution_time;
    this->_model->add_message(timestamp, message.str(), {job->_id});


    /* add AtlasEndAction */
    this->add_end_action();
}

template<typename T>
int AtlasEndScheduleAction<T>::time() const {
    int time_left_at_start =
        this->_schedule->job()->execution_time_left(this->_schedule->last_data()._begin);
    if (this->_schedule->last_data()._scheduler == AtlasSchedulerType::CFS) {
        time_left_at_start *= this->_model->_cfs_factor;
    }
    int time = this->_schedule->last_data()._begin
               + std::min(time_left_at_start, this->_schedule->last_data()._execution_time);

    /* check if there is still time left when there should not. This means there are dependencies */
    if (this->_schedule->job()->execution_time_left(time)) {
        time = this->_schedule->last_data()._begin + this->_schedule->last_data()._execution_time;
    }
    return time;
}

template<>
int AtlasEndScheduleAction<EarlyCfsSchedule>::time() const {
    int timestamp = this->_model->_timestamp;

    /* calculate left execution time */
    int execution_time_left = this->_schedule->job()->execution_time_left(timestamp);
    execution_time_left *= this->_model->_cfs_factor;

    /* calculate begin of corresponding atlas schedule */
    BaseAtlasSchedule *next_atlas_schedule =
        this->_model->next_atlas_schedule(this->_schedule->_core);
    int next_atlas_begin = next_atlas_schedule->last_data()._begin;

    int expected_end = std::min(timestamp + execution_time_left, next_atlas_begin);

    if (this->_schedule->atlas_job()->all_dependencies_finished(timestamp)) {
        return expected_end;
    }

    /* calculate end of dependent schedule.
     * Since the AtlasEndScheduleAction action moves the start in case of not running because of
     * dependencies, we have to set the End time to the actual beginning time */
    for (AtlasJob *dependency: this->_schedule->atlas_job()->dependencies()) {
        BaseAtlasSchedule *running_schedule = dependency->schedule_at_time(timestamp);
        if (running_schedule != nullptr) {
            return running_schedule->last_data().end();
        }
    }
    return next_atlas_begin;

}

template<>
void AtlasEndScheduleAction<EarlyCfsSchedule>::execute() {
    AtlasJob *job = this->_schedule->atlas_job();
    int timestamp = this->_model->_timestamp;

    /* check if execution is finished */
    if (job->execution_time_left(this->_model->_timestamp) <= 0) {

        /* create a change object that deletes the ATLAS schedule */
        this->_schedule->_atlas_schedule->add_change_delete(timestamp);
        if (this->_schedule->last_data().end() > timestamp) {
            this->_schedule->add_change_end(timestamp, timestamp);
        }

        /* there is place for a new schedule */
        std::cerr << timestamp << ": Early CFS Schedule for job " << job->_id << " finished"
                  << std::endl;
        this->_model->_actions_to_do.push_back(
            new AtlasFillAction(this->_model, timestamp, this->_schedule->_core));
    }

    if (this->_schedule->last_data().end() > timestamp) {
        this->_schedule->add_change_end(timestamp, timestamp);
    }
    /* schedule no longer executes */
    if (this->_model->_cfs_schedule[this->_schedule->_core] == this->_schedule) {
        this->_model->_cfs_schedule[this->_schedule->_core] = nullptr;
    }
}

template<>
void AtlasEndScheduleAction<LateCfsSchedule>::execute() {
    /* check if execution is finished */
    if (this->_schedule->job()->execution_time_left(this->_model->_timestamp) <= 0) {
        /* there is place for a new schedule */
        std::cerr << this->_model->_timestamp << ": Late CFS Schedule for job "
                  << this->_schedule->job()->_id << " finished" << std::endl;
        this->_model->_cfs_queue[this->_schedule->_core].pop_front();
        this->_model->_actions_to_do.push_back(
            new AtlasFillAction(this->_model, this->_model->_timestamp,
                                this->_schedule->_core));
    }

    /* schedule no longer executes */
    if (this->_model->_cfs_schedule[this->_schedule->_core] == this->_schedule) {
        this->_model->_cfs_schedule[this->_schedule->_core] = nullptr;
    }
}

template<>
void AtlasEndScheduleAction<RecoverySchedule>::execute() {
    int timestamp = this->_model->_timestamp;
    AtlasJob *job = this->_schedule->atlas_job();
    int time_left = job->execution_time_left(timestamp);
    int estimated_time_left = job->estimated_execution_time_left(timestamp);
    /* check if schedule finished */
    if (time_left <= 0) {
        std::cerr << timestamp << ": Recovery schedule ended for job " << job->_id << std::endl;
        AtlasScheduleData data = this->_schedule->last_data();
        if (timestamp < data._begin + data._execution_time) {
            /* adjust recovery schedule in model */
            this->_schedule->add_change_end(timestamp, timestamp);
        }
        /* schedule no longer executes */
        this->_model->_recovery_schedule[this->_schedule->_core] = nullptr;
        this->_model->_recovery_queue[this->_schedule->_core].pop_front();
    /* check if schedule was underestimated */
    } else if (estimated_time_left <= 0) {
        std::stringstream message;
        message << "Recovery schedule ended for job " << job->_id
                << " but job was underestimated. Job gets queued for CFS (late)";
        this->_model->add_message(timestamp, message.str(), {job->_id});

        this->_model->_cfs_queue[this->_schedule->_core].push_back(job);
    } else {
        std::stringstream message;
        message << "Recovery schedule ended for job " << job->_id
                << " but had not got all of its atlas time. Job stays on recovery.";
        this->_model->add_message(timestamp, message.str(), {job->_id});

        /* schedule no longer executes */
        this->_model->_recovery_schedule[this->_schedule->_core] = nullptr;
    }
    this->_model->_actions_to_do.push_back(
        new AtlasFillAction(this->_model, timestamp, this->_schedule->_core));
}

template<>
void AtlasEndScheduleAction<AtlasSchedule>::execute() {
    AtlasJob *job = this->_schedule->atlas_job();
    int timestamp = this->_model->_timestamp;

    /* check if ATLAS schedule still exists */
    if (job->execution_time_left(this->_schedule->last_data()._begin) <= 0) {
        return;
    }

    int time_left = job->execution_time_left(timestamp);
    int estimated_time_left = job->estimated_execution_time_left(timestamp);
    if (time_left <= 0) {
        std::string early = estimated_time_left > 0 ? "early " : "";
        std::cerr << this->_model->_timestamp
                  << ": Atlas schedule ended " << early << "for job " << job->_id
                  << std::endl;
        AtlasScheduleData data = this->_schedule->last_data();
        if (timestamp < data._begin + data._execution_time) {
            /* adjust atlas schedule in model */
            this->_schedule->add_change_end(timestamp, timestamp);
        }
    } else if (estimated_time_left <= 0) {
        std::stringstream message;
        message << "ATLAS schedule ended for job " << job->_id
                << " but Job was underestimated. Job gets queued for CFS (late).";
        this->_model->add_message(timestamp, message.str(), {job->_id});

        this->_model->_cfs_queue[this->_schedule->_core].push_back(this->_schedule->atlas_job());
    } else {
        std::stringstream message;
        if (not job->all_dependencies_finished(timestamp)) {
            message << "ATLAS schedule ended for job " << job->_id
                    << " but there were unknown dependencies. Job gets queued for Recovery.";
            job->_atlas_schedule->add_change_delete(timestamp);
        } else {
            message << "ATLAS schedule ended for job " << job->_id
                    << " but it had not got all of its atlas time. Job gets queued for Recovery.";
        }
        this->_model->add_message(timestamp, message.str(), {job->_id});

        /* do not show schedules that did not run */
        this->_schedule->end_simulation(timestamp);

        this->_model->_recovery_queue[this->_schedule->_core].push_back(this->_schedule->atlas_job());
    }
    this->_model->_actions_to_do.push_back(
        new AtlasFillAction(this->_model, timestamp, this->_schedule->_core));
}
