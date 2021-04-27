#include <simulation/rr_core_assigner.h>

void RoundRobinCoreAssigner::init(unsigned n_cores) {
    CoreAssigner::init(n_cores);
}

void RoundRobinCoreAssigner::init_assignment(std::vector<BaseJob *> jobs) {
    (void) jobs;
}

unsigned RoundRobinCoreAssigner::get_core_for_job(BaseJob *job) {
    (void) job;
    unsigned core = this->_next;
    this->_next = (this->_next + 1) % this->_n_cores;

    return core;
}
