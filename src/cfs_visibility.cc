#include <cfs_visibility.h>

bool Cfs_visibility::is_active_at_time(float timestamp) const {
    return (this->begin <= timestamp && this->end > timestamp);
}
