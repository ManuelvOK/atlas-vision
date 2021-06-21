#pragma once

#include <cbs/cbs_schedule.h>
#include <job.h>

class CbsJob: public Job<CbsSchedule> {
    using Job<CbsSchedule>::Job;
  public:
    virtual ~CbsJob() = default;
};

class HardRtJob: public CbsJob {
  public:
    typedef HardRtSchedule ScheduleType;
    using CbsJob::CbsJob;

    std::string to_string() const override;
};

class ConstantBandwidthServer;
class SoftRtJob: public CbsJob {
    std::map<int, int> _deadlines;
  public:
    typedef SoftRtSchedule ScheduleType;

    ConstantBandwidthServer *_cbs = nullptr;

    SoftRtJob(unsigned id, unsigned execution_time, int submission_time)
        : CbsJob(id, 0, execution_time, submission_time) {}

    int deadline(int timestamp) const;

    void add_change_deadline(int timestamp, int deadline);

    std::string to_string() const override;
};
