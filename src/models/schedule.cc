#include <models/schedule.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <models/job.h>


int Schedule::_next_id = 0;

static bool same_data(const ScheduleData &a, const ScheduleData &b) {
    return a._scheduler == b._scheduler
           and a._begin == b._begin
           and a._execution_time == b._execution_time;
}

Schedule::Schedule(int id, Job *job, int core, SchedulerType scheduler, int submission_time, int begin,
                   int execution_time) :
    _id(id),
    _job(job),
    _core(core),
    _submission_time(submission_time) {
    Schedule::_next_id = std::max(Schedule::_next_id, id + 1);
    this->_data.emplace(submission_time,
                        ScheduleData{scheduler, begin, execution_time});
    if (this->_job) {
        this->_job_id = this->_job->_id;
    }
}
Schedule::Schedule(int id, int job_id, int core, SchedulerType scheduler, int submission_time,
                   int begin, int execution_time) :
    Schedule(id, nullptr, core, scheduler, submission_time, begin, execution_time) {
    this->_job_id = job_id;
}


Schedule::Schedule(Job *job, int core, SchedulerType scheduler, int submission_time, int begin,
                   int execution_time) :
    Schedule(Schedule::next_id(), job, core, scheduler, submission_time, begin, execution_time) {}

Schedule::Schedule(const Schedule *s) :
    _id(Schedule::next_id()),
    _job(s->_job),
    _core(s->_core),
    _submission_time(s->_submission_time),
    _data(s->_data) {
    Schedule::_next_id = std::max(Schedule::_next_id, this->_id + 1);
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

void Schedule::add_change(int timestamp, int begin, int execution_time) {
    ScheduleData last_data = this->last_data();
    last_data._execution_time = execution_time;
    last_data._begin = begin;
    this->_data.emplace(timestamp, last_data);
}

void Schedule::add_change_shift_relative(int timestamp, int shift) {
    ScheduleData last_data = this->last_data();
    last_data._begin += shift;
    this->_data.emplace(timestamp, last_data);

}

void Schedule::add_change_execution_time_relative(int timestamp, int execution_time_difference) {
    ScheduleData last_data = this->last_data();
    last_data._execution_time += execution_time_difference;
    this->_data.emplace(timestamp, last_data);
}

void Schedule::add_change_end(int timestamp, int end) {
    ScheduleData last_data = this->last_data();
    last_data._execution_time = end - last_data._begin;
    this->_data.emplace(timestamp, last_data);
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

ScheduleData Schedule::first_data() const {
    ScheduleData data = this->_data.begin()->second;
    return data;
}

ScheduleData Schedule::last_data() const {
    ScheduleData data = this->_data.rbegin()->second;
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

std::string Schedule::to_string() const {
    ScheduleData data = this->_data.begin()->second;
    // > s job_id core scheduler submission_time begin execution_time
    std::stringstream ss;
    ss << "s " << this->_id
        << " " << this->_job->_id
        << " " << this->_core
        << " " << static_cast<char>(data._scheduler)
        << " " << this->_submission_time
        << " " << data._begin
        << " " << data._execution_time
        << " # id=" << this->_id
        << std::endl;

    /* write back changes */
    auto data_before = this->_data.begin();
    for (auto current_data = data_before; current_data != this->_data.end(); ++data_before,
                                                                             ++current_data) {
        char key = '\0';
        int value = 0;
        if (current_data->second._begin != data_before->second._begin) {
            key = 'b';
            value = current_data->second._begin;
        } else if (current_data->second._execution_time != data_before->second._execution_time) {
            key = 'e';
            value = current_data->second._execution_time;
        }

        ss << "a " << key
           << " " << value
           << " " << this->_id
           << std::endl;
    }
    if (this->_end > 0) {
        ss << "a d"
           << " " << this->_end
           << " " << this->_id
           << std::endl;
    }

    return ss.str();
}

int Schedule::next_id() {
    return Schedule::_next_id++;
}

void Schedule::reset_next_id() {
    Schedule::_next_id = 0;
}

CfsSchedule::CfsSchedule(AtlasSchedule *s, int submission_time, int begin, int execution_time) :
    Schedule(s) {
    this->_submission_time = submission_time;
    this->_data.clear();
    this->_data.emplace(submission_time,
                        ScheduleData{SchedulerType::CFS, begin, execution_time});
}

RecoverySchedule::RecoverySchedule(const Schedule *s) :
    Schedule(s) {
    for (auto &data: this->_data) {
        data.second._scheduler = SchedulerType::recovery;
    }
}
