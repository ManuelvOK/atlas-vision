#include <schedule.h>

#include <iostream>

Schedule::Schedule(int id, int job_id, int core, char scheduler, int submission_time, int begin,
        int execution_time)
    : id(id), job_id(job_id), core(core), scheduler(), submission_time(submission_time), begin(), execution_time() {
    this->scheduler.emplace(submission_time, static_cast<SchedulerType>(scheduler));
    this->begin.emplace(submission_time, begin);
    this->execution_time.emplace(submission_time, execution_time);
}

std::tuple<int, SchedulerType, int> Schedule::get_data_at_time(int timestamp) const {
    int begin_index = this->submission_time;
    for (std::pair<int, int> b: this->begin) {
        if (timestamp < b.first) {
            break;
        }
        begin_index = b.first;
    }

    /* since the scheduler is not an int, we cannot directly set it and have to use the index */
    int scheduler_index = submission_time;
    for (std::pair<int, SchedulerType> s: this->scheduler) {
        if (timestamp < s.first) {
            break;
        }
        scheduler_index = s.first;
    }

    int execution_time_index = submission_time;
    for (std::pair<int, int> e: this->execution_time) {
        if (timestamp < e.first) {
            break;
        }
        execution_time_index = e.first;
    }
    return {this->begin.at(begin_index), this->scheduler.at(scheduler_index),
            this->execution_time.at(execution_time_index)};
}

bool Schedule::exists_at_time(int timestamp) const {
    return (this->end < 0 || this->end > timestamp) && timestamp >= this->submission_time;
}

bool Schedule::is_active_at_time(int timestamp) const {
    int begin;
    int execution_time;
    SchedulerType scheduler;
    std::tie(begin, scheduler, execution_time) = this->get_data_at_time(timestamp);

    return (begin <= timestamp && begin + execution_time >= timestamp);
}

int Schedule::get_maximal_end() const {
    int begin = 0;
    for (std::pair<int, int> b: this->begin) {
        begin = std::max(begin, b.second);
    }
    /* I guess the execution time will not get worse for now
     * TODO: check this. */
    return begin + this->execution_time.at(this->submission_time);
}
