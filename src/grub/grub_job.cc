#include <grub/grub_job.h>

#include <grub/grub_constant_bandwidth_server.h>

std::string HardGrubJob::to_string() const {
    std::stringstream ss;

    ss << "j " << this->_id << " " << this->_deadline << " " << this->_execution_time
       << " " << this->_execution_time << " " << this->_submission_time
       << " -" << std::endl;

    for (GrubSchedule *schedule: this->_schedules) {
        ss << schedule->to_string() << std::endl;
    }
    return ss.str();
}

int SoftGrubJob::deadline(int timestamp) const {
    int deadline = 0;
    for (const auto &[t, dl]: this->_deadlines) {
        if (t > timestamp) {
            break;
        }
        deadline = dl;
    }
    return deadline;
}

void SoftGrubJob::add_change_deadline(int timestamp, int deadline) {
    this->_deadlines[timestamp] = deadline;
    this->_deadline = deadline;
}

std::string SoftGrubJob::to_string() const {
    std::stringstream ss;
    int deadline = 0;
    for (const auto &[_,job]: this->_cbs->jobs()) {
        deadline += this->_cbs->period();
        if (job == this) {
            break;
        }
    }
    ss << "j " << this->_id << " " << deadline //<< " " << this->_cbs->max_budget()
       << " " << this->_execution_time << " " << this->_submission_time
       << " " << this->_cbs->id() << std::endl;

    for (GrubSchedule *schedule: this->_schedules) {
        ss << schedule->to_string();
    }
    return ss.str();
}

