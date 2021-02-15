#pragma once

#include <controllers/core_assigner.h>

class RoundRobinCoreAssigner : public CoreAssigner {
    unsigned _next = 0;
public:
    RoundRobinCoreAssigner() = default;
    virtual void init(unsigned n_cores) override;
    virtual void init_assignment(std::vector<Job *> jobs) override;
    virtual unsigned get_core_for_job(Job *job) override;
};
