#include <model/cfs_visibility.h>

bool CfsVisibility::is_active_at_time(int timestamp) const {
    return (this->_begin <= timestamp && this->_end > timestamp);
}
