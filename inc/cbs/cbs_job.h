#pragma once

#include <cbs/cbs_schedule.h>
#include <job.h>

class CbsJob: public Job<CbsSchedule> {
    using Job<CbsSchedule>::Job;
};

class HardRtJob: public CbsJob {
  public:
    typedef HardRtSchedule ScheduleType;
    using CbsJob::CbsJob;
};

class ConstantBandwidthServer;
class SoftRtJob: public CbsJob {
    std::map<unsigned, unsigned> _deadlines;
  public:
    typedef SoftRtSchedule ScheduleType;

    ConstantBandwidthServer *_cbs = nullptr;

    SoftRtJob(unsigned id, unsigned execution_time, unsigned submission_time)
        : CbsJob(id, 0, execution_time, submission_time) {}

    unsigned deadline(unsigned timestamp) const;

    void add_change_deadline(unsigned timestamp, unsigned deadline);
};
