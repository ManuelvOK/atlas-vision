#include <atlas/atlas_simulation_action.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

#include <atlas/atlas_simulation_model.h>

static void end_schedule(BaseAtlasSchedule *schedule, AtlasSimulationModel *atlas_model);

void AtlasSubmissionAction::execute() {
    int timestamp = this->_model->_timestamp;
    unsigned core = this->_core_assigner->get_core_for_job(this->_job);

    /* stop cfs if running */
    end_schedule(this->_model->_cfs_schedule[core], this->_model);

    unsigned space = this->_model->space_on_atlas(timestamp, this->_job->_deadline);
    /* no space left on atlas before deadline */
    if (not space) {
        /* add to recovery and queue fillAction */
        this->_model->_recovery_queue[core].push_back(this->_job);
        this->_model->_actions_to_do.push_back(
            new AtlasFillAction(this->_model, this->_model->_timestamp, core));

        std::stringstream message;
        message << "Experimental: no time at all to schedule job " << this->_job->_id
            << " on core " << core << " on Atlas. -> Recovery_queue!";
        this->_model->add_message(timestamp, message.str(), {this->_job->_id});

        return;
    }

    unsigned atlas_time = this->_job->_execution_time_estimate;
    /* not enogh time on atlas */
    if (space < this->_job->_execution_time_estimate) {
        /* reduce atlas time to available space */
        atlas_time = space;

        std::stringstream message;
        message << "Experimental: no enough time to schedule job " << this->_job->_id
            << " on core " << core << " on Atlas. reduced time to " << atlas_time;
        this->_model->add_message(timestamp, message.str(), {this->_job->_id});
    }

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
            int end = data.end();
            if (end > max_end) {
                int shift_value = end - max_end;
                job->_atlas_schedule->add_change_shift_relative(timestamp, -shift_value);
                std::cout << "Schedule for Job " << job->_id << " shifted by " << -shift_value
                          << "." << std::endl;
            }
            max_end = std::min(max_end, job->_atlas_schedule->last_data()._begin);
            this->_model->resort_schedules();
            continue;
        }

        int begin = max_end - atlas_time;

        /* create a new schedule */
        AtlasSchedule *s = new AtlasSchedule (job, timestamp, core, begin, atlas_time);
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
                << " ATLAS from " << begin << " for " << atlas_time << ".";
        this->_model->add_message(timestamp, message.str(), {job->_id});
        max_end = std::min(max_end, begin);

    }


    /* check if there is an active schedule on CFS and if not, queue action to fill CFS */
    if (not this->_model->active_schedule_on_scheduler(core, AtlasSchedulerType::CFS,
                                                             this->_model->_timestamp)
        and not this->_model->active_schedule_on_scheduler(core, AtlasSchedulerType::ATLAS,
                                                                 this->_model->_timestamp)) {
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

    /* get next possible recovery job */
    AtlasJob *recovery_job = nullptr;

    if (not this->_model->_recovery_queue[this->_core].empty()) {
        for (AtlasJob *job: this->_model->_recovery_queue[this->_core]) {
            if (job->all_known_dependencies_finished(timestamp)) {
                recovery_job = job;
                break;
            }
        }
    }

    /* Check if there is something to execute on recovery */
    if (recovery_job) {
        /* Create Schedule on recovery and queue end action */
        unsigned time_left = recovery_job->estimated_execution_time_left(timestamp);
        BaseAtlasSchedule *next_atlas_schedule =
            this->_model->next_atlas_schedule(this->_core);
        unsigned time_to_next_atlas = time_left;
        if (next_atlas_schedule) {
            if (timestamp > next_atlas_schedule->last_data()._begin) {
                std::cout << "Error: next atlas is in past. PANIC!" << std::endl;
                exit(1);
            }
            time_to_next_atlas = next_atlas_schedule->last_data()._begin - timestamp;
        }
        unsigned recovery_time = std::min(time_left, time_to_next_atlas);
        RecoverySchedule *recovery_schedule =
            new RecoverySchedule(recovery_job, timestamp, this->_core, timestamp, recovery_time);

        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, recovery_job, recovery_schedule});

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
        unsigned execution_time = job->execution_time_left(timestamp) * this->_model->_cfs_factor;
        int next_begin = next_atlas_schedule->last_data()._begin;
        if (timestamp <= next_begin) {
            execution_time = std::min<unsigned>(execution_time, next_begin - timestamp);
        }
        EarlyCfsSchedule *cfs_schedule = new EarlyCfsSchedule(next_atlas_schedule, timestamp,
                                                              timestamp, execution_time);

        this->_model->_actions_to_do.push_back(
            new AtlasBeginScheduleAction{this->_model, cfs_schedule->atlas_job(), cfs_schedule});
    }

}

void end_schedule(BaseAtlasSchedule *schedule, AtlasSimulationModel *atlas_model) {
    if (not schedule) {
        return;
    }
    int timestamp = atlas_model->_timestamp;
    std::cerr << timestamp << ": End '" << int(schedule->last_data()._scheduler)
              << "' schedule on core " << schedule->_core
              << " for job " << schedule->job()->_id << std::endl;
    schedule->add_change_end(timestamp, timestamp);
    schedule->end_simulation(timestamp);

    /* schedule no longer executes */
    if (atlas_model->_cfs_schedule[schedule->_core] == schedule) {
        atlas_model->_cfs_schedule[schedule->_core] = nullptr;
    }
    if (atlas_model->_recovery_schedule[schedule->_core] == schedule) {
        atlas_model->_recovery_schedule[schedule->_core] = nullptr;
    }
}

template<typename T>
void AtlasBeginScheduleAction<T>::add_end_action() const {
    this->_model->_actions_to_do.push_back(
        new AtlasEndScheduleAction{this->_model, this->_job, this->_schedule});
}

template<typename T>
int AtlasBeginScheduleAction<T>::time() const {
    return this->_schedule->last_data()._begin;
}

template<>
void AtlasBeginScheduleAction<LateCfsSchedule>::execute() {
    int timestamp = this->_model->_timestamp;
    this->_schedule->atlas_job()->_schedules.push_back(this->_schedule);
    this->_model->add_cfs_schedule(this->_schedule);

    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    unsigned time_left = this->_schedule->job()->execution_time_left(timestamp);
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
    unsigned time_left = this->_schedule->job()->execution_time_left(timestamp);
    message << "Execute job " << this->_schedule->job()->_id << " on core "
            << this->_schedule->_core << " CFS (early). Time left: "
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
    end_schedule(this->_model->_cfs_schedule[this->_schedule->_core], this->_model);

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
    end_schedule(this->_model->_cfs_schedule[this->_schedule->_core], this->_model);
    end_schedule(this->_model->_recovery_schedule[this->_schedule->_core], this->_model);

    /* check dependencies */
    AtlasJob *dependent_job = job;
    std::vector<AtlasJob *> dependencies =
        dependent_job->not_finished_known_dependencies(timestamp);
    unsigned dependency_time_left = 0;
    /* aggregate time that all the dependent jobs in the dependency chain have left to run */
    while (dependencies.size() > 0) {
        dependent_job = dependencies[0];
        unsigned time_left = dependent_job->execution_time_left(timestamp);
        dependencies = dependent_job->not_finished_known_dependencies(timestamp);
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
        message << "job " << job->_id << " depends on job " << dependent_job->_id
                << ". Inserting ATLAS dependency schedule";
        this->_model->add_message(timestamp, message.str(), {job->_id, dependent_job->_id});
        unsigned length = std::min(this->_schedule->last_data()._execution_time, dependency_time_left);

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
            this->_schedule->add_change_does_execute(timestamp, false);
            this->_schedule->add_change_delete(this->_schedule->last_data().end());
            this->_model->_recovery_queue[this->_schedule->_core].push_back(this->_schedule->atlas_job());
            std::cerr << timestamp << ": there's no time for job " << job->_id
                      << " after added schedule finished. -> Recovery" << std::endl;
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
    unsigned time_left_at_start =
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
    unsigned execution_time_left = this->_schedule->job()->execution_time_left(timestamp);
    execution_time_left *= this->_model->_cfs_factor;

    if (not execution_time_left) {
        return timestamp;
    }

    /* calculate begin of corresponding atlas schedule */
    BaseAtlasSchedule *next_atlas_schedule =
        this->_model->next_atlas_schedule(this->_schedule->_core);
    int next_atlas_begin = next_atlas_schedule->last_data()._begin;

    int expected_end = std::min(timestamp + static_cast<int>(execution_time_left),
                                next_atlas_begin);

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
        std::cerr << timestamp << ":\tset end to now." << std::endl;
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
    unsigned time_left = job->execution_time_left(timestamp);
    unsigned estimated_time_left = job->estimated_execution_time_left(timestamp);
    /* check if schedule finished */
    if (time_left <= 0) {
        std::cerr << timestamp << ": Recovery schedule ended for job " << job->_id << std::endl;
        AtlasScheduleData data = this->_schedule->last_data();
        if (timestamp < data._begin + static_cast<int>(data._execution_time)) {
            /* adjust recovery schedule in model */
            this->_schedule->add_change_end(timestamp, timestamp);
        }
        /* schedule no longer executes */
        this->_model->_recovery_schedule[this->_schedule->_core] = nullptr;
        this->_model->_recovery_queue[this->_schedule->_core].remove(job);
    /* check if schedule was underestimated */
    } else if (estimated_time_left <= 0) {
        std::stringstream message;
        message << "Recovery schedule ended for job " << job->_id
                << " but job was underestimated. Job gets queued for CFS (late)";
        this->_model->add_message(timestamp, message.str(), {job->_id});

        /* schedule no longer executes */
        this->_model->_recovery_schedule[this->_schedule->_core] = nullptr;

        this->_model->_recovery_queue[this->_schedule->_core].remove(job);
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

    unsigned time_left = job->execution_time_left(timestamp);
    unsigned estimated_time_left = job->estimated_execution_time_left(timestamp);
    if (time_left <= 0) {
        std::string early = estimated_time_left > 0 ? "early " : "";
        std::cerr << this->_model->_timestamp
                  << ": Atlas schedule ended " << early << "for job " << job->_id
                  << std::endl;
        AtlasScheduleData data = this->_schedule->last_data();
        if (timestamp < data._begin + static_cast<int>(data._execution_time)) {
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
