#pragma once

#include <schedule.h>

struct CbsScheduleData : public ScheduleData {

};

class CbsJob;
class CbsSchedule : public Schedule<CbsScheduleData> {
  protected:
    CbsJob *_cbs_job;
  public:
    CbsSchedule(unsigned id, CbsJob *job, int submission_time, unsigned core, int begin,
                unsigned execution_time);
    CbsSchedule(CbsJob *job, int submission_time, unsigned core, int begin,
                unsigned execution_time);

    CbsJob *cbs_job() const;

    std::string to_string() const override;
};

class HardRtJob;
class HardRtSchedule : public CbsSchedule {
  public:
    HardRtJob *_rt_job;

    HardRtSchedule(HardRtJob *job, int submission_time, unsigned core, int begin,
                   unsigned execution_time);
};

class SoftRtJob;
class SoftRtSchedule : public CbsSchedule {
  public:
    SoftRtJob *_rt_job;

    SoftRtSchedule(SoftRtJob *job, int submission_time, unsigned core, int begin,
                   unsigned execution_time);

    GuiScheduleData get_vision_data_at_time(int timestamp) const override;
};
