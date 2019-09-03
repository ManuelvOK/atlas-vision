#include <model/schedule.h>

#include <iostream>

Schedule::Schedule(int id, int job_id, int core, char scheduler, int submission_time, int begin,
        int execution_time)
    : _id(id), _job_id(job_id), _core(core), _scheduler(), _submission_time(submission_time), _begin(), _execution_time() {
    this->_scheduler.emplace(submission_time, static_cast<SchedulerType>(scheduler));
    this->_begin.emplace(submission_time, begin);
    this->_execution_time.emplace(submission_time, execution_time);
}

std::tuple<int, SchedulerType, int> Schedule::get_data_at_time(int timestamp) const {
    int begin_index = this->_submission_time;
    for (std::pair<int, int> b: this->_begin) {
        if (timestamp < b.first) {
            break;
        }
        begin_index = b.first;
    }

    /* since the scheduler is not an int, we cannot directly set it and have to use the index */
    int scheduler_index = this->_submission_time;
    for (std::pair<int, SchedulerType> s: this->_scheduler) {
        if (timestamp < s.first) {
            break;
        }
        scheduler_index = s.first;
    }

    int execution_time_index = this->_submission_time;
    for (std::pair<int, int> e: this->_execution_time) {
        if (timestamp < e.first) {
            break;
        }
        execution_time_index = e.first;
    }
    return {this->_begin.at(begin_index), this->_scheduler.at(scheduler_index),
            this->_execution_time.at(execution_time_index)};
}

bool Schedule::exists_at_time(int timestamp) const {
    return (this->_end < 0 || this->_end > timestamp) && timestamp >= this->_submission_time;
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
    for (std::pair<int, int> b: this->_begin) {
        begin = std::max(begin, b.second);
    }
    /* I guess the execution time will not get worse for now
     * TODO: check this. */
    return begin + this->_execution_time.at(this->_submission_time);
}
