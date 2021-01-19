#include <models/cfs_visibility.h>

#include <sstream>

bool CfsVisibility::is_active_at_time(int timestamp) const {
    return (this->_begin <= timestamp && this->_end > timestamp);
}

std::string CfsVisibility::to_string() const {
    std::stringstream ss;
    ss << "v " << this->_schedule_id
       << " "  << this->_begin
       << " "  << this->_end
       << std::endl;
    return ss.str();
}
