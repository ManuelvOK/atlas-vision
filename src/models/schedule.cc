#include <models/schedule.h>

#include <algorithm>
#include <iostream>

static bool same_data(const ScheduleData &a, const ScheduleData &b) {
    return a._scheduler == b._scheduler
           and a._begin == b._begin
           and a._execution_time == b._execution_time;
}

Schedule::Schedule(int id, int job_id, int core, char scheduler, int submission_time, int begin,
                   int execution_time) :
    _id(id),
    _job_id(job_id),
    _core(core),
    _submission_time(submission_time) {
    this->_data.emplace(submission_time,
                        ScheduleData{static_cast<SchedulerType>(scheduler), begin, execution_time});
}

void Schedule::add_change(const ScheduleChange *change) {
    /* get last data before change */
    ScheduleData &old_data = this->data_at_time(change->_timestamp);
    if (change->_type == ChangeType::erase) {
        this->_end = change->_timestamp;
        /* add information that the schedule does not run because of unknown dependencies */
        old_data._does_execute = false;
        return;
    }
    /* copy old data */
    ScheduleData new_data = old_data;
    switch (change->_type) {
        case ChangeType::shift:
            new_data._begin = change->_value;
            break;
        case ChangeType::change_execution_time:
            new_data._execution_time = change->_value;
            break;
        default: break;
    }
    if (not same_data(old_data, new_data)) {
        this->_data.emplace(change->_timestamp, new_data);
        this->_change_points.insert(change->_timestamp);
    }
}

ScheduleData &Schedule::data_at_time(int timestamp) {
    /* find dataset for given timestamp */
    int data_index = this->_submission_time;
    for (std::pair<int, ScheduleData> d: this->_data) {
        if (timestamp < d.first) {
            break;
        }
        data_index = d.first;
    }
    return this->_data.at(data_index);
}

const ScheduleData &Schedule::data_at_time(int timestamp) const {
    /* find dataset for given timestamp */
    int data_index = this->_submission_time;
    for (std::pair<int, ScheduleData> d: this->_data) {
        if (timestamp < d.first) {
            break;
        }
        data_index = d.first;
    }
    return this->_data.at(data_index);
}

ScheduleData Schedule::get_data_at_time(int timestamp) const {

    ScheduleData data = this->data_at_time(timestamp);

    if (data._scheduler == SchedulerType::CFS) {
        data._execution_time = std::max(0,std::min(data._execution_time, timestamp - data._begin));
    }
    if (not data._does_execute and timestamp >= data._begin) {
        data._begin = timestamp + 1;
    }
    return data;
}

bool Schedule::exists_at_time(int timestamp) const {
    return (this->_end < 0 || this->_end > timestamp) && timestamp >= this->_submission_time;
}

bool Schedule::is_active_at_time(int timestamp) const {
    ScheduleData data = this->get_data_at_time(timestamp);

    return (data._begin <= timestamp && data._begin + data._execution_time >= timestamp);
}

int Schedule::get_maximal_end() const {
    auto max_data = std::max_element(this->_data.begin(), this->_data.end(),
        [](const std::pair<int, ScheduleData> &a, const std::pair<int, ScheduleData> &b){
            return a.second._begin < b.second._begin;
        });
    ScheduleData data = max_data->second;
    /* I guess the execution time will not get worse for now
     * TODO: check this. */
    return data._begin + data._execution_time;
}
