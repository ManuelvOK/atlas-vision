#include <models/simulation_action.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

#include <models/atlas_model.h>

int TimedAction::time() const {
    return this->_time;
}

void SubmissionAction::action() {
    /* add scheduled jobs to the list of jobs to schedule */
    std::vector<Job *> scheduled_jobs = this->_atlas_model->next_atlas_scheduled_jobs();
    this->_jobs.insert(this->_jobs.end(), scheduled_jobs.begin(), scheduled_jobs.end());
    /* sort jobs EDF wise while considering dependencies */
    std::sort(this->_jobs.begin(), this->_jobs.end(), [=](const Job *a, const Job *b) {
        return b->depends_on(a) or (not a->depends_on(b) and a->_deadline < b->_deadline);
    });

    /* iterate through the EDF sorted jobs in reversed order to add schedules while minimizing
     * slack. */
    int max_end = std::numeric_limits<int>::max();
    std::cerr << this->_atlas_model->_timestamp << ": adjust or create Schedules ============"
              << std::endl;
    for (auto job_it = this->_jobs.rbegin(); job_it != this->_jobs.rend(); ++job_it) {
        Job *job = *job_it;
        max_end = std::min(max_end, job->_deadline);

        /* if there already is a schedule, we might have to adjust it */
        if (job->_atlas_schedule != nullptr) {
            ScheduleData data = job->_atlas_schedule->last_data();
            int shift_value = max_end - data.end();
            if (shift_value < 0) {
                job->_atlas_schedule->add_change_shift_relative(this->_atlas_model->_timestamp,
                                                                shift_value);
                std::cerr << this->_atlas_model->_timestamp << ": Schedule for Job " << job->_id
                          << " shifted by " << shift_value << "." << std::endl;
            } else {
                std::cerr << this->_atlas_model->_timestamp << ": Schedule for Job " << job->_id
                          << " need not be shifted. Max_end: " << max_end << " schedule_end: "
                          << data.end() << " schedule_begin: " << data._begin << "." << std::endl;
            }
            max_end = std::min(max_end, job->_atlas_schedule->last_data()._begin);
            continue;
        }

        /* create a new schedule */
        int begin = max_end - job->_execution_time;
        AtlasSchedule *s = new AtlasSchedule (job, 0, this->_atlas_model->_timestamp, begin,
                                              job->_execution_time);
        job->set_atlas_schedule(s);
        this->_atlas_model->add_atlas_schedule(s);

        /* queue an action to begin the schedule. */
        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, s});

        /* queue an action for the deadline of the job */
        this->_atlas_model->_actions_to_do.push_back(
            new DeadlineAction(this->_atlas_model, s->_job->_deadline, s->_job));
        std::cerr << this->_atlas_model->_timestamp << ": Job " << job->_id
                  << " submitted and scheduled on ATLAS from " << begin << " for "
                  << job->_execution_time << ". Max_end was " << max_end << "." << std::endl;
        max_end = std::min(max_end, begin);

    }


    /* check if there is an active schedule on CFS and if not, queue action to fill CFS */
    if (not this->_atlas_model->active_schedule_on_scheduler(SchedulerType::CFS, this->_atlas_model->_timestamp)
        and not this->_atlas_model->active_schedule_on_scheduler(SchedulerType::ATLAS, this->_atlas_model->_timestamp)) {
        std::cerr << this->_atlas_model->_timestamp << ": CFS is free and has to be filled."
                  << std::endl;
        this->_atlas_model->_actions_to_do.push_back(
            new FillAction(this->_atlas_model, this->_atlas_model->_timestamp));
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
    /* check whether there is an active schedule anywhere. If so, do nothing */
    if (this->_atlas_model->active_schedule(this->_atlas_model->_timestamp)) {
        auto s = this->_atlas_model->active_schedule(this->_atlas_model->_timestamp);
        std::cerr << this->_atlas_model->_timestamp << ": tried to fill but job "
                  << s->_job->_id << " was runing until "
                  << s->last_data()._begin + s->last_data()._execution_time << std::endl;
        return;
    }
    this->_atlas_model->tidy_up_queues();

    /* Check if there is something on recovery */
    if (not this->_atlas_model->_recovery_queue.empty()) {
        /* Create Schedule on recovery and queue end action */
        Job *job = this->_atlas_model->_recovery_queue.front();

        RecoverySchedule *recovery_schedule =
            new RecoverySchedule(job, 0, this->_atlas_model->_timestamp,
                                 this->_atlas_model->_timestamp,
                                 job->execution_time_left(this->_atlas_model->_timestamp));

        job->_schedules.push_back(recovery_schedule);
        std::cerr << this->_atlas_model->_timestamp << ": fill recovery with job " << job->_id
                  << ". Time left: " << recovery_schedule->last_data()._execution_time << std::endl;

        this->_atlas_model->_recovery_schedules.push_back(recovery_schedule);
        this->_atlas_model->_schedules.insert(recovery_schedule);

        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, recovery_schedule});

    } else if (not this->_atlas_model->_cfs_queue.empty()) {
        Job *job = this->_atlas_model->_cfs_queue.front();
        /* Create Schedule on cfs and queue end action */
        LateCfsSchedule *cfs_schedule =
            new LateCfsSchedule(job, 0, this->_atlas_model->_timestamp,
                                this->_atlas_model->_timestamp,
                                job->execution_time_left(this->_atlas_model->_timestamp)
                                * this->_atlas_model->_cfs_factor);
        job->_schedules.push_back(cfs_schedule);
        std::cerr << this->_atlas_model->_timestamp << ": fill cfs from queue with job "
                  << job->_id << ". Time left: " << cfs_schedule->last_data()._execution_time
                  << std::endl;
        /* add cfs schedule to atlas_model */
        this->_atlas_model->_cfs_schedules.push_back(cfs_schedule);
        this->_atlas_model->_schedules.insert(cfs_schedule);

        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, cfs_schedule});
    } else {
        /* find next schedule on ATLAS */
        AtlasSchedule *next_atlas_schedule = this->_atlas_model->next_atlas_schedule();
        /* if there is none, abort */
        if (not next_atlas_schedule) {
            return;
        }
        std::cerr << this->_atlas_model->_timestamp << ": next atlas schedule found for job "
                  << next_atlas_schedule->_job->_id << std::endl;

        /* create schedule on CFS based on next atlas schedule and adjust it */
        int execution_time =
            next_atlas_schedule->_job->execution_time_left(this->_atlas_model->_timestamp)
            * this->_atlas_model->_cfs_factor;
        EarlyCfsSchedule *cfs_schedule = new EarlyCfsSchedule(next_atlas_schedule,
                                                              this->_atlas_model->_timestamp,
                                                              this->_atlas_model->_timestamp,
                                                              execution_time);

        cfs_schedule->_job->_schedules.push_back(cfs_schedule);
        std::cerr << this->_atlas_model->_timestamp << ": fill cfs with job "
                  << cfs_schedule->_job->_id << ". Time left: "
                  << cfs_schedule->last_data()._execution_time << std::endl;

        /* add cfs schedule to atlas_model */
        this->_atlas_model->_cfs_schedules.push_back(cfs_schedule);
        this->_atlas_model->_schedules.insert(cfs_schedule);

        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, cfs_schedule});
    }

}

template<typename T>
void BeginScheduleAction<T>::endSchedule(Schedule *schedule) {
    if (not schedule) {
        return;
    }
    schedule->add_change_end(this->_atlas_model->_timestamp,
                             this->_atlas_model->_timestamp);
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
    std::cerr << this->_atlas_model->_timestamp << ": Beginning Late CFS Schedule for job "
              << this->_schedule->_job->_id << ". time left: "
              << this->_schedule->last_data()._execution_time << std::endl;

    this->_atlas_model->_cfs_schedule = this->_schedule;
    /* add EndAction */
    this->add_end_action();
}

template<>
void BeginScheduleAction<EarlyCfsSchedule>::action() {

    this->_atlas_model->_cfs_schedule = this->_schedule;
    /* add EndAction */
    this->add_end_action();
}

template<>
void BeginScheduleAction<RecoverySchedule>::action() {
    /* CONTINUE: check dependencies ?!? */

    /* stop cfs if running */
    this->endSchedule(this->_atlas_model->_cfs_schedule);

    this->_atlas_model->_recovery_schedule = this->_schedule;
    /* add EndAction */
    this->add_end_action();
}

template<>
void BeginScheduleAction<AtlasSchedule>::action() {
    Job *job = this->_schedule->_job;
    /* check if ATLAS schedule still exists */
    if (job->execution_time_left(this->_atlas_model->_timestamp) <= 0) {
        return;
    }
    std::cerr << this->_atlas_model->_timestamp << ": begin ATLAS Schedule for job "
              << job->_id << ". time left: "
              << job->execution_time_left(this->_atlas_model->_timestamp) << std::endl;

    /* stop everything thats running */
    this->endSchedule(this->_atlas_model->_cfs_schedule);
    this->endSchedule(this->_atlas_model->_recovery_schedule);

    /* check dependencies */
    /* TODO: check for all dependencies, not only the first one. this was quick and dirty */
    Job *dependent_job = job;
    int dependency_time_left = 0;
    /* aggregate time that all the dependent jobs in the dependency chain have left to run */
    while (dependent_job->_known_dependencies.size() > 0) {
        int time_left = dependent_job->_known_dependencies[0]
            ->execution_time_left(this->_atlas_model->_timestamp);
        if (time_left <= 0) {
            break;
        }
        dependent_job = dependent_job->_known_dependencies[0];
        dependency_time_left = time_left;
    }
    if (dependent_job != job) {
        std::cerr << this->_atlas_model->_timestamp << ": job " << job->_id
                  << " depends on job " << dependent_job->_id
                  << ". Inserting ATLAS dependency schedule" << std::endl;
        int length = std::min(this->_schedule->last_data()._execution_time, dependency_time_left);
        DependencySchedule *schedule =
            new DependencySchedule(dependent_job, 0, this->_atlas_model->_timestamp,
                                   this->_atlas_model->_timestamp, length);
        dependent_job->_schedules.push_back(schedule);
        this->_atlas_model->add_atlas_schedule(schedule);
        this->_atlas_model->_actions_to_do.push_back(
            new BeginScheduleAction{this->_atlas_model, schedule});
        if (this->_schedule->last_data()._execution_time > dependency_time_left) {
            /* dependency schedule comes to an end before schedule would finish */
            std::cerr << this->_atlas_model->_timestamp << ": theres time for job " << job->_id
                      << " after added schedule finished." << std::endl;
            this->_schedule->add_change_shift_relative(this->_atlas_model->_timestamp, length);
            this->_schedule->add_change_execution_time_relative(this->_atlas_model->_timestamp,
                                                                -dependency_time_left);
            this->_atlas_model->_actions_to_do.push_back(
                new BeginScheduleAction{this->_atlas_model, this->_schedule});
        } else {
            this->_schedule->add_change_delete(this->_atlas_model->_timestamp);
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
    return this->_schedule->last_data()._begin
           + std::min(time_left_at_start, this->_schedule->last_data()._execution_time);
}

template<>
void EndScheduleAction<EarlyCfsSchedule>::action() {
    Job *job = this->_schedule->_job;
    int timestamp = this->_atlas_model->_timestamp;
    /* check if there was any execution */
    if (not job->all_dependencies_finished(timestamp)) {
        std::cerr << timestamp << ": Cfs Schedule ended for job " << job->_id
                  << ". But there were unknown dependencies. => Deleting schedule." << std::endl;
        this->_schedule->add_change_delete(timestamp);
    }

    /* check if execution is finished */
    if (job->execution_time_left(this->_atlas_model->_timestamp) <= 0) {

        /* create a change object that deletes the ATLAS schedule */
        this->_schedule->_atlas_schedule->add_change_delete(timestamp);

        /* there is place for a new schedule */
        std::cerr << timestamp << ": Early CFS Schedule for job " << job->_id << " finished"
                  << std::endl;
        this->_atlas_model->_actions_to_do.push_back(new FillAction(this->_atlas_model, timestamp));
    }
    /* visibility stays until now */
    std::cerr << timestamp << ": visibility ended for job " << job->_id << std::endl;
    ScheduleData data = this->_schedule->last_data();
    this->_atlas_model->_cfs_visibilities.push_back(
        new CfsVisibility(this->_schedule->_atlas_schedule, data._begin,
                          data._begin + data._execution_time));
    /* schedule no longer executes */
    if (this->_atlas_model->_cfs_schedule == this->_schedule) {
        this->_atlas_model->_cfs_schedule = nullptr;
    }
}

template<>
void EndScheduleAction<LateCfsSchedule>::action() {
    /* check if execution is finished */
    if (this->_schedule->_job->execution_time_left(this->_atlas_model->_timestamp) <= 0) {
        /* there is place for a new schedule */
        std::cerr << this->_atlas_model->_timestamp << ": Late CFS Schedule for job "
                  << this->_schedule->_job->_id << " finished" << std::endl;
        this->_atlas_model->_cfs_queue.pop_front();
        this->_atlas_model->_actions_to_do.push_back(
            new FillAction(this->_atlas_model, this->_atlas_model->_timestamp));
    }

    /* schedule no longer executes */
    if (this->_atlas_model->_cfs_schedule == this->_schedule) {
        this->_atlas_model->_cfs_schedule = nullptr;
    }
}

template<>
void EndScheduleAction<RecoverySchedule>::action() {
    int time_left = this->_schedule->_job->execution_time_left(this->_atlas_model->_timestamp);
    int estimated_time_left =
        this->_schedule->_job->estimated_execution_time_left(this->_atlas_model->_timestamp);
    if (time_left <= 0) {
        std::cerr << this->_atlas_model->_timestamp
                  << ": Recovery schedule ended (early) for job " << this->_schedule->_job->_id
                  << std::endl;
        if (this->_atlas_model->_timestamp < this->_schedule->last_data()._begin
                                                  + this->_schedule->last_data()._execution_time) {
            /* adjust recovery schedule in atlas_model */
            this->_schedule->add_change_end(this->_atlas_model->_timestamp,
                                            this->_atlas_model->_timestamp);
        }
        /* schedule no longer executes */
        this->_atlas_model->_recovery_schedule = nullptr;
        this->_atlas_model->_recovery_queue.pop_front();
    } else if (estimated_time_left <= 0) {
        std::cerr << this->_atlas_model->_timestamp << ": Recovery schedule ended for job "
                  << this->_schedule->_job->_id << " but job was underestimated. => CFS."
                  << std::endl;
        this->_atlas_model->_cfs_queue.push_back(this->_schedule->_job);
    } else {
        std::cerr << this->_atlas_model->_timestamp << ": Recovery schedule ended for job "
                  << this->_schedule->_job->_id
                  << " but schedule had not got all of its atlas time. => Staying on Recovery."
                  << std::endl;
        /* schedule no longer executes */
        this->_atlas_model->_recovery_schedule = nullptr;
    }
    this->_atlas_model->_actions_to_do.push_back(
        new FillAction(this->_atlas_model, this->_atlas_model->_timestamp));

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
        std::cerr << timestamp << ": Atlas schedule ended for job " << job->_id
                  << " but job was underestimated. => CFS." << std::endl;
        this->_atlas_model->_cfs_queue.push_back(this->_schedule->_job);
    } else {
        if (not job->all_dependencies_finished(timestamp)) {
            std::cerr << timestamp << ": Atlas schedule ended for job " << job->_id
                      << ". But there were unknown dependencies. => Recovery." << std::endl;
            job->_atlas_schedule->add_change_delete(timestamp);
        } else {
            std::cerr << timestamp << ": Atlas schedule ended for job " << job->_id
                      << ". But schedule had not got all of its atlas time. => Recovery." << std::endl;
        }
        this->_atlas_model->_recovery_queue.push_back(this->_schedule->_job);
    }
    this->_atlas_model->_actions_to_do.push_back(
        new FillAction(this->_atlas_model, timestamp));
}
