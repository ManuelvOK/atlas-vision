#include <cbs/cbs_job.h>

#include <cbs/constant_bandwidth_server.h>

std::string HardRtJob::to_string() const {
    std::stringstream ss;
    ss << "r " << this->_id << " " << this->_deadline << " " << this->_execution_time
       << " " << this->_submission_time << std::endl;

    for (CbsSchedule *schedule: this->_schedules) {
        ss << schedule->to_string() << std::endl;
    }
    return ss.str();
}

unsigned SoftRtJob::deadline(unsigned timestamp) const {
    unsigned deadline = 0;
    for (const auto &[t, dl]: this->_deadlines) {
        if (t > timestamp) {
            break;
        }
        deadline = dl;
    }
    return deadline;
}

void SoftRtJob::add_change_deadline(unsigned timestamp, unsigned deadline) {
    this->_deadlines[timestamp] = deadline;
    this->_deadline = deadline;
}

std::string SoftRtJob::to_string() const {
    std::stringstream ss;
    ss << "j " << this->_id << " " << this->_execution_time << " " << this->_submission_time
       << " " << this->_cbs->id() << std::endl;

    for (CbsSchedule *schedule: this->_schedules) {
        ss << schedule->to_string();
    }
    return ss.str();
}
