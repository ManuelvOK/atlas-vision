#pragma once

#include <simulation/core_assigner.h>

class RoundRobinCoreAssigner : public CoreAssigner {
    unsigned _next = 0;
public:
    RoundRobinCoreAssigner() = default;
    virtual void init(unsigned n_cores) override;
    virtual void init_assignment(std::vector<BaseJob *> jobs) override;
    virtual unsigned get_core_for_job(BaseJob *job) override;
};
