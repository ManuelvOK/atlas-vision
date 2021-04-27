#include <schedule.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <job.h>
#include <util/parser.h>

int ScheduleData::end() const {
    return this->_begin + this->_execution_time;
}


int BaseSchedule::_next_id = 0;

BaseSchedule::BaseSchedule(int id, BaseJob *job, int submission_time, unsigned core)
    : _job(job), _id(id), _submission_time(submission_time), _core(core) {
    BaseSchedule::_next_id = std::max(BaseSchedule::_next_id, this->_id + 1);
}

int BaseSchedule::next_id() {
    return BaseSchedule::_next_id++;
}

void BaseSchedule::reset_next_id() {
    BaseSchedule::_next_id = 0;
}

BaseJob *BaseSchedule::job() const {
    return this->_job;
}

bool BaseSchedule::exists_at_time(int timestamp) const {
    return (this->_end < 0 || this->_end > timestamp) && timestamp >= this->_submission_time;
}


//std::string Schedule::to_string() const {
//    ScheduleData data = this->_data.begin()->second;
//    // > s job_id core scheduler submission_time begin execution_time
//    std::stringstream ss;
//    ss << "s " << this->_id
//        << " " << this->_job->_id
//        << " " << this->_core
//        << " " << static_cast<char>(data._scheduler)
//        << " " << this->_submission_time
//        << " " << data._begin
//        << " " << data._execution_time
//        << " # id=" << this->_id
//        << std::endl;
//
//    /* write back changes */
//    auto data_before = this->_data.begin();
//    for (auto current_data = data_before; current_data != this->_data.end(); ++data_before,
//                                                                             ++current_data) {
//        char key = '\0';
//        int value = 0;
//        if (current_data->second._begin != data_before->second._begin) {
//            key = 'b';
//            value = current_data->second._begin;
//        } else if (current_data->second._execution_time != data_before->second._execution_time) {
//            key = 'e';
//            value = current_data->second._execution_time;
//        }
//
//        ss << "a " << key
//           << " " << value
//           << " " << this->_id
//           << std::endl;
//    }
//    if (this->_end > 0) {
//        ss << "a d"
//           << " " << this->_end
//           << " " << this->_id
//           << std::endl;
//    }
//
//    return ss.str();
//}
