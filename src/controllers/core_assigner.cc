#include <controllers/core_assigner.h>

void CoreAssigner::init(unsigned n_cores) {
    this->_initialised = true;
    this->_n_cores = n_cores;
}
