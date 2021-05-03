#include <cbs/cbs_job.h>

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
