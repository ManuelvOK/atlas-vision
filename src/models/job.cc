#include <models/job.h>

#include <iostream>

int Job::calculate_dependency_level() {
    /* only calculate if already set */
    if (this->_dependency_level >= 0) {
        return this->_dependency_level;
    }
    if (this->_known_dependencies.size() == 0 && this->_unknown_dependencies.size() == 0) {
        this->_dependency_level = 0;
        return 0;
    }
    this->_dependency_level = 1;
    for (Job *j: this->_known_dependencies) {
        this->_dependency_level = std::max(this->_dependency_level, 1 + j->calculate_dependency_level());
    }
    for (Job *j: this->_unknown_dependencies) {
        this->_dependency_level = std::max(this->_dependency_level, 1 + j->calculate_dependency_level());
    }
    return this->_dependency_level;
}
