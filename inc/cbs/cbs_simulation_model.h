#pragma once

#include <cbs/cbs_job.h>
#include <cbs/cbs_schedule.h>
#include <simulation/simulation_model.h>

class CbsSimulationModel : public SimulationModel<CbsSchedule, CbsJob> {
  public:
    std::vector<HardRtSchedule *> _hard_rt_schedules;
    std::vector<SoftRtSchedule *> _soft_rt_schedules;

    std::map<unsigned, std::list<CbsJob *>> _soft_rt_queue;

    void add_hard_rt_schedule(HardRtSchedule *schedule);
    void add_soft_rt_schedule(SoftRtSchedule *schedule);
};
