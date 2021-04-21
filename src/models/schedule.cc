#include <models/schedule.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <models/cfs_visibility.h>
#include <models/job.h>
#include <util/parser.h>


int Schedule::_next_id = 0;

static bool same_data(const ScheduleData &a, const ScheduleData &b) {
    return a._scheduler == b._scheduler
           and a._begin == b._begin
           and a._execution_time == b._execution_time;
}

int ScheduleData::end() const {
    return this->_begin + this->_execution_time;
}

Schedule::Schedule(int id, Job *job, unsigned core, SchedulerType scheduler, int submission_time,
                   int begin, int execution_time, bool end_known) :
    _id(id),
    _job(job),
    _core(core),
    _submission_time(submission_time) {
    Schedule::_next_id = std::max(Schedule::_next_id, id + 1);
    this->_data.emplace(submission_time,
                        ScheduleData{submission_time, scheduler, begin, execution_time, false,
                        end_known});
}

Schedule::Schedule(Job *job, unsigned core, SchedulerType scheduler, int submission_time, int begin,
                   int execution_time, bool end_known) :
    Schedule(Schedule::next_id(), job, core, scheduler, submission_time, begin, execution_time,
             end_known) {}

Schedule::Schedule(const Schedule *s) :
    _id(Schedule::next_id()),
    _job(s->_job),
    _core(s->_core),
    _submission_time(s->_submission_time),
    _data(s->_data) {
    Schedule::_next_id = std::max(Schedule::_next_id, this->_id + 1);
}

EarlyCfsSchedule::EarlyCfsSchedule(AtlasSchedule *s, int submission_time, int begin,
                                   int execution_time)
    : CfsSchedule(s, submission_time, begin, execution_time),
    _atlas_schedule(s) {
    //this->_visibility = new CfsVisibility(this, begin, this->_atlas_schedule->last_data()._begin);
}

CfsVisibility EarlyCfsSchedule::create_visibility() const {
    return CfsVisibility(this->_atlas_schedule, this->first_data()._begin,
                         std::min(this->last_data().end(),
                                  this->_atlas_schedule->last_data()._begin));
}

void Schedule::add_change(const ParsedChange &change) {
    /* get last data before change */
    ScheduleData &old_data = this->data_at_time(change._timestamp);
    ChangeType type = static_cast<ChangeType>(change._type);
    if (type == ChangeType::erase) {
        this->_end = change._timestamp;
        /* add information that the schedule does not run because of unknown dependencies */
        old_data._does_execute = false;
        return;
    }
    /* copy old data */
    ScheduleData new_data = old_data;
    new_data._timestamp = change._timestamp;
    switch (type) {
        case ChangeType::shift:
            new_data._begin = change._value;
            break;
        case ChangeType::change_execution_time:
            new_data._execution_time = change._value;
            break;
        default: break;
    }
    if (not same_data(old_data, new_data)) {
        this->_data.emplace(change._timestamp, new_data);
        this->_change_points.insert(change._timestamp);
    }
}

void Schedule::add_change(int timestamp, int begin, int execution_time) {
    ScheduleData new_data = this->last_data();
    new_data._timestamp = timestamp;
    new_data._execution_time = execution_time;
    new_data._begin = begin;
    this->_data[timestamp] = new_data;
}

void Schedule::add_change_begin(int timestamp, int begin, bool did_execute) {
    ScheduleData &old_data = this->last_data();
    ScheduleData new_data = old_data;
    new_data._timestamp = timestamp;
    new_data._begin = begin;
    if (not did_execute) {
        old_data._does_execute = false;
    }
    this->_data[timestamp] = new_data;
}

void Schedule::add_change_does_execute(int timestamp, bool does_execute) {
    ScheduleData new_data = this->last_data();
    new_data._timestamp = timestamp;
    new_data._does_execute = does_execute;
    this->_data[timestamp] = new_data;
}

void Schedule::add_change_end_known(int timestamp, bool end_known) {
    ScheduleData new_data = this->last_data();
    new_data._timestamp = timestamp;
    new_data._end_known = end_known;
    this->_data[timestamp] = new_data;
}

void Schedule::add_change_shift_relative(int timestamp, int shift) {
    ScheduleData new_data = this->last_data();
    new_data._timestamp = timestamp;
    new_data._begin += shift;
    this->_data[timestamp] = new_data;

}

void Schedule::add_change_execution_time_relative(int timestamp, int execution_time_difference) {
    ScheduleData new_data = this->last_data();
    new_data._timestamp = timestamp;
    new_data._execution_time += execution_time_difference;
    this->_data[timestamp] = new_data;
}

void Schedule::add_change_end(int timestamp, int end) {
    ScheduleData new_data = this->last_data();
    new_data._timestamp = timestamp;
    new_data._execution_time = end - new_data._begin;
    this->_data[timestamp] = new_data;
}

void Schedule::add_change_delete(int timestamp) {
    ScheduleData last_data = this->last_data();

    /* when CFS schedules get deleted there has been an end change before */
    if (last_data._timestamp == timestamp) {
        this->_data.erase(last_data._timestamp);
    }
    ScheduleData &old_data = this->last_data();

    this->_end = timestamp;
    ScheduleData new_data = old_data;
    old_data._does_execute = false;
    new_data._execution_time = 0;
    this->_data[timestamp] = new_data;
}

void Schedule::end_simulation(int timestamp) {
    this->_simulation_ended = true;
    ScheduleData data = this->last_data();
    if (data._execution_time == 0 or not data._does_execute) {
        this->_end = timestamp;
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

ScheduleData Schedule::get_vision_data_at_time(int timestamp) const {

    ScheduleData data = this->data_at_time(timestamp);

    if (not data._end_known) {
        data._execution_time = std::max(0,std::min(data._execution_time, timestamp - data._begin));
    }
    if (not data._does_execute and timestamp >= data._begin) {
        data._execution_time = std::max(0, data.end() - timestamp);
        data._begin = timestamp; // + 1;
    }
    return data;
}

ScheduleData Schedule::get_data_at_time(int timestamp) const {
    return this->data_at_time(timestamp);
}

ScheduleData Schedule::first_data() const {
    ScheduleData data = this->_data.begin()->second;
    return data;
}

ScheduleData &Schedule::last_data() {
    ScheduleData &data = this->_data.rbegin()->second;
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

    return (data._begin <= timestamp && data._begin + data._execution_time > timestamp);
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
                        ScheduleData{submission_time, SchedulerType::CFS, begin, execution_time, false, false});
}

RecoverySchedule::RecoverySchedule(const Schedule *s) :
    Schedule(s) {
    for (auto &data: this->_data) {
        data.second._scheduler = SchedulerType::recovery;
    }
}
