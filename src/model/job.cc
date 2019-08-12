#include <model/job.h>

#include <iostream>

int Job::calculate_dependency_level() {
    /* only calculate if already set */
    if (this->dependency_level >= 0) {
        std::cerr << "level of job " << this->id << " already calculated" << std::endl;
        return this->dependency_level;
    }
    if (this->known_dependencies.size() == 0) {
        this->dependency_level = 0;
        return 0;
    }
    this->dependency_level = 1;
    for (Job *j: this->known_dependencies) {
        this->dependency_level = std::max(this->dependency_level, 1 + j->calculate_dependency_level());
    }
    return this->dependency_level;
}
