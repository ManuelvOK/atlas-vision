#pragma once

#include <schedule.h>

struct CbsScheduleData : public ScheduleData {

};

class CbsJob;
class CbsSchedule : public Schedule<CbsScheduleData> {
  protected:
    CbsJob *_cbs_job;
  public:
    CbsSchedule(unsigned id, CbsJob *job, unsigned submission_time, unsigned core,
                unsigned execution_time, unsigned begin);
    CbsSchedule(CbsJob *job, unsigned submission_time, unsigned core, unsigned execution_time,
                unsigned begin);

    CbsJob *cbs_job() const;
};

class HardRtJob;
class HardRtSchedule : public CbsSchedule {
  public:
    HardRtJob *_rt_job;

    HardRtSchedule(HardRtJob *job, int submission_time, unsigned core, unsigned execution_time,
                   unsigned begin);
};

class SoftRtJob;
class SoftRtSchedule : public CbsSchedule {
  public:
    SoftRtJob *_rt_job;

    SoftRtSchedule(SoftRtJob *job, int submission_time, unsigned core, unsigned execution_time,
                   unsigned begin);
};
