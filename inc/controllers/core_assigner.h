#pragma once

#include <vector>

#include <models/job.h>

class CoreAssigner {
protected:
    bool _initialised = false;
    unsigned _n_cores = 0;
    CoreAssigner() = default;
public:
    virtual ~CoreAssigner() = default;
    virtual void init(unsigned n_cores);
    virtual void init_assignment(std::vector<Job *> jobs) = 0;
    virtual unsigned get_core_for_job(Job *job) = 0;
};
