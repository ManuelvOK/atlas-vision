#include <job.h>

#include <iostream>
#include <sstream>

unsigned BaseJob::deadline(unsigned timestamp) const {
    (void) timestamp;
    return this->_deadline;
}

/* amount of time to still be executed */
unsigned BaseJob::execution_time_left(unsigned timestamp) const {
    return this->_execution_time - this->time_executed(timestamp);
}

bool BaseJob::finished(unsigned timestamp) const {
    return this->execution_time_left(timestamp) <= 0;
}

//std::string BaseJob::to_string() const {
//    std::stringstream ss;
//    ss << "j " << this->_id
//       << " " << this->_deadline
//       << " " << this->_execution_time_estimate
//       << " " << this->_execution_time
//       << " " << this->_submission_time
//       << " # id=" << this->_id
//       << std::endl;
//
//    for (BaseJob *j: this->_known_dependencies) {
//        ss << "d k "
//            << this->_id
//            << " " << j->_id
//            << std::endl;
//    }
//    for (BaseJob *j: this->_unknown_dependencies) {
//        ss << "d u "
//            << this->_id
//            << " " << j->_id
//            << std::endl;
//    }
//    return ss.str();
//}
