#include <models/simulation_action.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

#include <models/atlas_model.h>

SimulationAction::SimulationAction(AtlasModel *atlas_model, int weight) :
    _atlas_model(atlas_model),
    _weight(weight) {}

int TimedAction::time() const {
    return this->_time;
}

void SubmissionAction::action() {
    int timestamp = this->_atlas_model->_timestamp;
    unsigned core = this->_core_assigner->get_core_for_job(this->_job);
    /* add scheduled jobs to the list of jobs to schedule */
    std::vector<Job *> jobs = this->_atlas_model->next_atlas_scheduled_jobs(core);
    jobs.push_back(this->_job);
    /* sort jobs EDF wise while considering dependencies */
    std::sort(jobs.begin(), jobs.end(), [=](const Job *a, const Job *b) {
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
        Job *job = *job_it;
        max_end = std::min(max_end, job->_deadline);

        /* if there already is a schedule, we might have to adjust it */
        if (job->_atlas_schedule != nullptr) {
            ScheduleData data = job->_atlas_schedule->last_data();
            int shift_value = max_end - data.end();
            if (shift_value < 0) {
                job->_atlas_schedule->add_change_shift_relative(timestamp,
                                                                shift_value);
                std::stringstream message;
                message << "Schedule for Job " << job->_id << " shifted by " << shift_value << ".";
                this->_atlas_model->add_message(timestamp, message.str());
            }
            max_end = std::min(max_end, job->_atlas_schedule->last_data()._begin);
            this->_atlas_model->resort_schedules();
            continue;
        }

        /* create a new schedule */
        int begin = max_end - job->_execution_time_estimate;
        AtlasSchedule *s = new AtlasSchedule (job, core, timestamp, begin,
                                              job->_execution_time_estimate);
        job->set_atlas_schedule(s);
        this->_atlas_model->add_atlas_schedule(s);

        /* queue an action to begin the schedule. */
        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, s});

        /* queue an action for the deadline of the job */
        this->_atlas_model->_actions_to_do.push_back(
            new DeadlineAction(this->_atlas_model, s->_job->_deadline, s->_job));

        std::stringstream message;
        message << "Job " << job->_id << " submitted and scheduled on core " << core
                << " ATLAS from " << begin << " for " << job->_execution_time_estimate << ".";
        this->_atlas_model->add_message(timestamp, message.str());
        max_end = std::min(max_end, begin);

    }


    /* check if there is an active schedule on CFS and if not, queue action to fill CFS */
    if (not this->_atlas_model->active_schedule_on_scheduler(core, SchedulerType::CFS,
                                                             this->_atlas_model->_timestamp)
        and not this->_atlas_model->active_schedule_on_scheduler(core, SchedulerType::ATLAS,
                                                                 this->_atlas_model->_timestamp)) {
        std::cerr << this->_atlas_model->_timestamp << ": CFS on core " << core
                  << " is free and has to be filled." << std::endl;
        this->_atlas_model->_actions_to_do.push_back(
            new FillAction(this->_atlas_model, this->_atlas_model->_timestamp, core));
    }
}


void DeadlineAction::action() {
    /* check if job has finished or if there is execution time left */
    if (this->_job->execution_time_left(this->_atlas_model->_timestamp) > 0) {
        std::cerr << this->_atlas_model->_timestamp << ": Deadline miss for job "
                  << this->_job->_id << std::endl;
    } else {
        std::cerr << this->_atlas_model->_timestamp << ": Deadline for job "
                  << this->_job->_id << std::endl;
    }
}

void FillAction::action() {
    int timestamp = this->_atlas_model->_timestamp;
    /* check whether there is an active schedule anywhere. If so, do nothing */
    auto s = this->_atlas_model->active_schedule(this->_core, timestamp);
    if (s) {
        std::cerr << timestamp << ": tried to fill core " << this->_core << " but job "
                  << s->_job->_id << " was runing until "
                  << s->last_data()._begin + s->last_data()._execution_time << std::endl;
        return;
    }
    this->_atlas_model->tidy_up_queues();

    /* Check if there is something on recovery */
    if (not this->_atlas_model->_recovery_queue[this->_core].empty()) {
        /* Create Schedule on recovery and queue end action */
        Job *job = this->_atlas_model->_recovery_queue[this->_core].front();

        int time_left = job->estimated_execution_time_left(timestamp);
        Schedule *next_atlas_schedule = this->_atlas_model->next_atlas_schedule(this->_core);
        int time_to_next_atlas = time_left;
        if (next_atlas_schedule) {
            time_to_next_atlas = next_atlas_schedule->last_data()._begin - timestamp;
        }
        int recovery_time = std::min(time_left, time_to_next_atlas);
        RecoverySchedule *recovery_schedule =
            new RecoverySchedule(job, this->_core, timestamp, timestamp, recovery_time);

        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, recovery_schedule});

    /* Check if there is something in CFS queue */
    } else if (not this->_atlas_model->_cfs_queue[this->_core].empty()) {
        Job *job = this->_atlas_model->_cfs_queue[this->_core].front();
        /* Create Schedule on cfs and queue end action */
        LateCfsSchedule *cfs_schedule =
            new LateCfsSchedule(job, this->_core, timestamp, timestamp,
                                job->estimated_execution_time_left(timestamp)
                                * this->_atlas_model->_cfs_factor);

        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, cfs_schedule});
    } else {
        /* find next schedule on ATLAS */
        AtlasSchedule *next_atlas_schedule = this->_atlas_model->next_atlas_schedule(this->_core);
        /* if there is none, abort */
        if (not next_atlas_schedule) {
            return;
        }
        Job *job = next_atlas_schedule->_job;
        std::cerr << timestamp << ": next atlas schedule found on core " << this->_core
                  << " for job " << job->_id << std::endl;

        /* create schedule on CFS based on next atlas schedule and adjust it */
        int execution_time = job->execution_time_left(timestamp) * this->_atlas_model->_cfs_factor;
        execution_time = std::min(execution_time, next_atlas_schedule->last_data()._begin);
        EarlyCfsSchedule *cfs_schedule = new EarlyCfsSchedule(next_atlas_schedule, timestamp,
                                                              timestamp, execution_time);

        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, cfs_schedule});
    }

}

template<typename T>
void BeginScheduleAction<T>::end_schedule(Schedule *schedule) {
    if (not schedule) {
        return;
    }
    int timestamp = this->_atlas_model->_timestamp;
    std::cerr << timestamp << ": End '" << (char)schedule->last_data()._scheduler
              << "' schedule on core " << schedule->_core
              << " for job " << schedule->_job->_id << std::endl;
    schedule->add_change_end(timestamp, timestamp);
    schedule->end_simulation(timestamp);

    /* schedule no longer executes */
    if (this->_atlas_model->_cfs_schedule[schedule->_core] == schedule) {
        this->_atlas_model->_cfs_schedule[schedule->_core] = nullptr;
    }
    if (this->_atlas_model->_recovery_schedule[schedule->_core] == schedule) {
        this->_atlas_model->_recovery_schedule[schedule->_core] = nullptr;
    }
}

template<typename T>
void BeginScheduleAction<T>::add_end_action() const {
    this->_atlas_model->_actions_to_do.push_back(
        new EndScheduleAction{this->_atlas_model, this->_schedule});
}

template<typename T>
int BeginScheduleAction<T>::time() const{
    return this->_schedule->last_data()._begin;
}

template<>
void BeginScheduleAction<LateCfsSchedule>::action() {
    this->_schedule->_job->_schedules.push_back(this->_schedule);
    this->_atlas_model->add_cfs_schedule(this->_schedule);

    this->_schedule->add_change_does_execute(this->_atlas_model->_timestamp, true);
    std::stringstream message;
    message << "Execute job " << this->_schedule->_job->_id << " on core "
            << this->_schedule->_core << " CFS (late). Time left: "
            << this->_schedule->last_data()._execution_time;
    this->_atlas_model->add_message(this->_atlas_model->_timestamp, message.str());

    this->_atlas_model->_cfs_schedule[this->_schedule->_core] = this->_schedule;
    /* add EndAction */
    this->add_end_action();
}

template<>
void BeginScheduleAction<EarlyCfsSchedule>::action() {
    int timestamp = this->_atlas_model->_timestamp;

    /* check if schedule has been ended by other event */
    if (this->_schedule->_simulation_ended) {
        return;
    }

    /* check if we already have tried to add this schedule */
    if (this->_atlas_model->_cfs_schedule[this->_schedule->_core] != this->_schedule) {
        this->_atlas_model->_cfs_schedule[this->_schedule->_core] = this->_schedule;
        this->_schedule->_job->_schedules.push_back(this->_schedule);
        this->_atlas_model->add_early_cfs_schedule(this->_schedule);
    }

    /* check if dependencies have finished */
    if (not this->_schedule->_job->all_dependencies_finished(timestamp)) {
        std::cerr << timestamp << ": Can not start cfs schedule on core "
                  << this->_schedule->_core << " for job " << this->_schedule->_job->_id
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
    message << "Execute job " << this->_schedule->_job->_id << " on core "
            << this->_schedule->_core << " CFS (early).";
    this->_atlas_model->add_message(this->_atlas_model->_timestamp, message.str());

    /* add EndAction */
    this->add_end_action();
}

template<>
void BeginScheduleAction<RecoverySchedule>::action() {
    int timestamp = this->_atlas_model->_timestamp;

    /* check if schedule has been ended by other event */
    if (this->_schedule->_simulation_ended) {
        return;
    }

    /* check if we already have tried to add this schedule */
    if (this->_atlas_model->_recovery_schedule[this->_schedule->_core] != this->_schedule) {
        this->_atlas_model->_recovery_schedule[this->_schedule->_core] = this->_schedule;
        this->_schedule->_job->_schedules.push_back(this->_schedule);
        this->_atlas_model->add_recovery_schedule(this->_schedule);
    }

    /* check if dependencies have finished */
    if (not this->_schedule->_job->all_dependencies_finished(timestamp)) {
        std::cerr << timestamp << ": Can not start recovery schedule on core "
                  << this->_schedule->_core << " for job " << this->_schedule->_job->_id
                  << " because of dependencies. Waiting..." << std::endl;
        this->_success = false;
        return;
    }

    /* update begin time if necessary */
    ScheduleData data = this->_schedule->last_data();
    if (data._begin != timestamp) {
        this->_schedule->add_change_begin(timestamp, timestamp, false);
        int next_atlas_schedule_begin =
            this->_atlas_model->next_atlas_schedule(this->_schedule->_core)
            ->last_data()._begin;
        data = this->_schedule->last_data();
        if (data.end() > next_atlas_schedule_begin) {
            this->_schedule->add_change_end(timestamp, next_atlas_schedule_begin);
        }
    }

    /* stop cfs if running */
    this->end_schedule(this->_atlas_model->_cfs_schedule[this->_schedule->_core]);

    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    message << "Execute job " << this->_schedule->_job->_id << " on core " << this->_schedule->_core
            << " recovery. Scheduled time: " << this->_schedule->last_data()._execution_time;
    this->_atlas_model->add_message(timestamp, message.str());

    this->_atlas_model->_recovery_schedule[this->_schedule->_core] = this->_schedule;
    /* add EndAction */
    this->add_end_action();
}

template<>
void BeginScheduleAction<AtlasSchedule>::action() {
    int timestamp = this->_atlas_model->_timestamp;
    Job *job = this->_schedule->_job;
    /* check if ATLAS schedule still exists */
    if (job->execution_time_left(timestamp) <= 0) {
        return;
    }
    /* check if there is a schedule already running */
    this->_schedule->add_change_does_execute(timestamp, true);
    std::stringstream message;
    message << "Execute job " << job->_id << " on core " << this->_schedule->_core
            << " ATLAS. Time left: " << this->_schedule->last_data()._execution_time;
    this->_atlas_model->add_message(timestamp, message.str());

    /* stop everything thats running */
    this->end_schedule(this->_atlas_model->_cfs_schedule[this->_schedule->_core]);
    this->end_schedule(this->_atlas_model->_recovery_schedule[this->_schedule->_core]);

    /* check dependencies */
    /* TODO: check for all dependencies, not only the first one. this was quick and dirty */
    Job *dependent_job = job;
    std::vector<Job *> dependencies = dependent_job->known_dependencies();
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
        std::cerr << timestamp << ": job " << job->_id << " depends on job " << dependent_job->_id
                  << ". Inserting ATLAS dependency schedule" << std::endl;
        int length = std::min(this->_schedule->last_data()._execution_time, dependency_time_left);
        DependencySchedule *schedule =
            new DependencySchedule(dependent_job, this->_schedule->_core, timestamp, timestamp, length);
        dependent_job->_schedules.push_back(schedule);
        this->_atlas_model->add_atlas_schedule(schedule);
        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, schedule});
        if (this->_schedule->last_data()._execution_time > dependency_time_left) {
            /* dependency schedule comes to an end before schedule would finish */
            std::cerr << timestamp << ": theres time for job " << job->_id
                      << " after added schedule finished." << std::endl;
            this->_schedule->add_change_shift_relative(timestamp, length);
            this->_schedule->add_change_execution_time_relative(timestamp, -dependency_time_left);
            this->_atlas_model->_actions_to_do.push_back(
                new BeginScheduleAction{this->_atlas_model, this->_schedule});
        } else {
            this->_schedule->add_change_delete(timestamp);
        }
        return;
    }

    /* add EndAction */
    this->add_end_action();
}

template<typename T>
int EndScheduleAction<T>::time() const {
    int time_left_at_start =
        this->_schedule->_job->execution_time_left(this->_schedule->last_data()._begin);
    if (this->_schedule->last_data()._scheduler == SchedulerType::CFS) {
        time_left_at_start *= this->_atlas_model->_cfs_factor;
    }
    int time = this->_schedule->last_data()._begin
               + std::min(time_left_at_start, this->_schedule->last_data()._execution_time);

    /* check if there is still time left when there should not. This means there are dependencies */
    if (this->_schedule->_job->execution_time_left(time)) {
        time = this->_schedule->last_data()._begin + this->_schedule->last_data()._execution_time;
    }
    return time;
}

template<>
int EndScheduleAction<EarlyCfsSchedule>::time() const {
    int timestamp = this->_atlas_model->_timestamp;

    /* calculate left execution time */
    int execution_time_left = this->_schedule->_job->execution_time_left(timestamp);
    execution_time_left *= this->_atlas_model->_cfs_factor;

    /* calculate begin of corresponding atlas schedule */
    Schedule *next_atlas_schedule = this->_atlas_model->next_atlas_schedule(this->_schedule->_core);
    int next_atlas_begin = next_atlas_schedule->last_data()._begin;

    int expected_end = std::min(timestamp + execution_time_left, next_atlas_begin);

    if (this->_schedule->_job->all_dependencies_finished(timestamp)) {
        return expected_end;
    }

    /* calculate end of dependent schedule.
     * Since the EndScheduleAction action moves the start in case of not running because of
     * dependencies, we have to set the End time to the actual beginning time */
    for (Job *dependency: this->_schedule->_job->dependencies()) {
        Schedule *running_schedule = dependency->schedule_at_time(timestamp);
        if (running_schedule != nullptr) {
            return running_schedule->last_data().end();
        }
    }
    return next_atlas_begin;

}

template<>
void EndScheduleAction<EarlyCfsSchedule>::action() {
    Job *job = this->_schedule->_job;
    int timestamp = this->_atlas_model->_timestamp;

    /* check if execution is finished */
    if (job->execution_time_left(this->_atlas_model->_timestamp) <= 0) {

        /* create a change object that deletes the ATLAS schedule */
        this->_schedule->_atlas_schedule->add_change_delete(timestamp);
        if (this->_schedule->last_data().end() > timestamp) {
            this->_schedule->add_change_end(timestamp, timestamp);
        }

        /* there is place for a new schedule */
        std::cerr << timestamp << ": Early CFS Schedule for job " << job->_id << " finished"
                  << std::endl;
        this->_atlas_model->_actions_to_do.push_back(
            new FillAction(this->_atlas_model, timestamp, this->_schedule->_core));
    }

    if (this->_schedule->last_data().end() > timestamp) {
        this->_schedule->add_change_end(timestamp, timestamp);
    }
    /* schedule no longer executes */
    if (this->_atlas_model->_cfs_schedule[this->_schedule->_core] == this->_schedule) {
        this->_atlas_model->_cfs_schedule[this->_schedule->_core] = nullptr;
    }
}

template<>
void EndScheduleAction<LateCfsSchedule>::action() {
    /* check if execution is finished */
    if (this->_schedule->_job->execution_time_left(this->_atlas_model->_timestamp) <= 0) {
        /* there is place for a new schedule */
        std::cerr << this->_atlas_model->_timestamp << ": Late CFS Schedule for job "
                  << this->_schedule->_job->_id << " finished" << std::endl;
        this->_atlas_model->_cfs_queue[this->_schedule->_core].pop_front();
        this->_atlas_model->_actions_to_do.push_back(
            new FillAction(this->_atlas_model, this->_atlas_model->_timestamp,
                           this->_schedule->_core));
    }

    /* schedule no longer executes */
    if (this->_atlas_model->_cfs_schedule[this->_schedule->_core] == this->_schedule) {
        this->_atlas_model->_cfs_schedule[this->_schedule->_core] = nullptr;
    }
}

template<>
void EndScheduleAction<RecoverySchedule>::action() {
    int timestamp = this->_atlas_model->_timestamp;
    Job *job = this->_schedule->_job;
    int time_left = job->execution_time_left(timestamp);
    int estimated_time_left = job->estimated_execution_time_left(timestamp);
    /* check if schedule finished */
    if (time_left <= 0) {
        std::cerr << timestamp << ": Recovery schedule ended for job " << job->_id << std::endl;
        ScheduleData data = this->_schedule->last_data();
        if (timestamp < data._begin + data._execution_time) {
            /* adjust recovery schedule in atlas_model */
            this->_schedule->add_change_end(timestamp, timestamp);
        }
        /* schedule no longer executes */
        this->_atlas_model->_recovery_schedule[this->_schedule->_core] = nullptr;
        this->_atlas_model->_recovery_queue[this->_schedule->_core].pop_front();
    /* check if schedule was underestimated */
    } else if (estimated_time_left <= 0) {
        std::stringstream message;
        message << "Recovery schedule ended for job " << job->_id
                << " but job was underestimated. Job gets queued for CFS (late)";
        this->_atlas_model->add_message(timestamp, message.str());

        this->_atlas_model->_cfs_queue[this->_schedule->_core].push_back(job);
    } else {
        std::stringstream message;
        message << "Recovery schedule ended for job " << job->_id
                << " but had not got all of its atlas time. Job stays on recovery.";
        this->_atlas_model->add_message(timestamp, message.str());

        /* schedule no longer executes */
        this->_atlas_model->_recovery_schedule[this->_schedule->_core] = nullptr;
    }
    this->_atlas_model->_actions_to_do.push_back(
        new FillAction(this->_atlas_model, timestamp, this->_schedule->_core));
}

template<>
void EndScheduleAction<AtlasSchedule>::action() {
    Job *job = this->_schedule->_job;
    int timestamp = this->_atlas_model->_timestamp;
    /* check if ATLAS schedule still exists */
    if (job->execution_time_left(this->_schedule->last_data()._begin) <= 0) {
        return;
    }
    int time_left = job->execution_time_left(timestamp);
    int estimated_time_left = job->estimated_execution_time_left(timestamp);
    if (time_left <= 0) {
        std::string early = estimated_time_left > 0 ? "early " : "";
        std::cerr << this->_atlas_model->_timestamp
                  << ": Atlas schedule ended " << early << "for job " << job->_id
                  << std::endl;
        ScheduleData data = this->_schedule->last_data();
        if (timestamp < data._begin + data._execution_time) {
            /* adjust atlas schedule in atlas_model */
            this->_schedule->add_change_end(timestamp, timestamp);
        }
    } else if (estimated_time_left <= 0) {
        std::stringstream message;
        message << "ATLAS schedule ended for job " << job->_id
                << " but Job was underestimated. Job gets queued for CFS (late).";
        this->_atlas_model->add_message(timestamp, message.str());

        this->_atlas_model->_cfs_queue[this->_schedule->_core].push_back(this->_schedule->_job);
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
        this->_atlas_model->add_message(timestamp, message.str());

        this->_atlas_model->_recovery_queue[this->_schedule->_core].push_back(this->_schedule->_job);
    }
    this->_atlas_model->_actions_to_do.push_back(
        new FillAction(this->_atlas_model, timestamp, this->_schedule->_core));
}
