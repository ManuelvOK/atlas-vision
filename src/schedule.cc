#include <schedule.h>

#include <iostream>

Schedule::Schedule(int id, int job_id, int core, char scheduler, float submission_time, float begin,
        float execution_time)
    : id(id), job_id(job_id), core(core), scheduler(), submission_time(submission_time), begin(), execution_time() {
    this->scheduler.emplace(submission_time, static_cast<scheduler_type>(scheduler));
    this->begin.emplace(submission_time, begin);
    this->execution_time.emplace(submission_time, execution_time);
}

std::tuple<float, scheduler_type, float> Schedule::get_data_at_time(float timestamp) const {
    float begin_index = this->submission_time;
    for (std::pair<float, float> b: this->begin) {
        if (timestamp < b.first) {
            break;
        }
        begin_index = b.first;
    }

    /* since the scheduler is not an int, we cannot directly set it and have to use the index */
    float scheduler_index = submission_time;
    for (std::pair<float, scheduler_type> s: this->scheduler) {
        if (timestamp < s.first) {
            break;
        }
        scheduler_index = s.first;
    }

    float execution_time_index = submission_time;
    for (std::pair<float, float> e: this->execution_time) {
        if (timestamp < e.first) {
            break;
        }
        execution_time_index = e.first;
    }
    return {this->begin.at(begin_index), this->scheduler.at(scheduler_index),
            this->execution_time.at(execution_time_index)};
}

bool Schedule::exists_at_time(float timestamp) const {
    return (this->end < 0 || this->end > timestamp) && timestamp >= this->submission_time;
}

float Schedule::get_maximal_end() const {
    float begin = 0;
    for (std::pair<float, float> b: this->begin) {
        begin = std::max(begin, b.second);
    }
    /* I guess the execution time will not get worse for now
     * TODO: check this. */
    return begin + this->execution_time.at(this->submission_time);
}
