#include <model/cfs_visibility.h>

bool CfsVisibility::is_active_at_time(int timestamp) const {
    return (this->begin <= timestamp && this->end > timestamp);
}
