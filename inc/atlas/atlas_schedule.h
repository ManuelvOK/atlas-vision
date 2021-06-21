#pragma once

#include <schedule.h>
#include <gui/schedule_rect.h>

class CfsVisibility;

/** type of scheduler encoded as ascii character */
enum class AtlasSchedulerType {
    ATLAS,
    recovery,
    CFS
};

struct AtlasScheduleData : public ScheduleData {
    AtlasSchedulerType _scheduler;   /**< the scheduler this schedule is on */
};

class AtlasJob;
class BaseAtlasSchedule : public Schedule<AtlasScheduleData> {
  protected:
    AtlasJob *_atlas_job;
  public:
    BaseAtlasSchedule(unsigned id, AtlasJob *job, int submission_time, unsigned core,
                      AtlasSchedulerType scheduler, int begin, unsigned execution_time,
                      bool end_known = true);
    BaseAtlasSchedule(AtlasJob *job, int submission_time, unsigned core,
                      AtlasSchedulerType scheduler, int begin, unsigned execution_time,
                      bool end_known = true);
    BaseAtlasSchedule(const BaseAtlasSchedule *s)
        : Schedule<AtlasScheduleData>(s), _atlas_job(s->_atlas_job) {}

    AtlasJob *atlas_job() const;
    GuiScheduleData get_vision_data_at_time(int timestamp) const override;

    std::string to_string() const override;
};

class AtlasSchedule : public BaseAtlasSchedule {
  public:
    using BaseAtlasSchedule::BaseAtlasSchedule;
    AtlasSchedule(unsigned id, AtlasJob *job, int submission_time, unsigned core,
                  int begin, unsigned execution_time, bool end_known = true)
        : BaseAtlasSchedule(id, job, submission_time, core, AtlasSchedulerType::ATLAS, begin,
                            execution_time, end_known) {}
    AtlasSchedule(AtlasJob *job, int submission_time, unsigned core, int begin,
                  unsigned execution_time, bool end_known = true)
        : BaseAtlasSchedule(job, submission_time, core, AtlasSchedulerType::ATLAS, begin,
                            execution_time, end_known) {}

};

class DependencySchedule : public AtlasSchedule {
  public:
    using AtlasSchedule::AtlasSchedule;
};

class CfsSchedule : public BaseAtlasSchedule {
  public:
    using BaseAtlasSchedule::BaseAtlasSchedule;
    CfsSchedule(unsigned id, AtlasJob *job, int submission_time, unsigned core, int begin,
                unsigned execution_time)
        : BaseAtlasSchedule(id, job, submission_time, core, AtlasSchedulerType::CFS, begin,
                            execution_time, false) {}
    CfsSchedule(AtlasJob *job, int submission_time, unsigned core, int begin,
                unsigned execution_time)
        : BaseAtlasSchedule(job, submission_time, core, AtlasSchedulerType::CFS, begin,
                            execution_time, false) {}
    CfsSchedule(AtlasSchedule *s, int submission_time, int begin, unsigned execution_time);
};

class EarlyCfsSchedule : public CfsSchedule {
  public:
    using CfsSchedule::CfsSchedule;
    AtlasSchedule *_atlas_schedule = nullptr;
    EarlyCfsSchedule(AtlasSchedule *s, int submission_time, int begin, unsigned execution_time);
    CfsVisibility create_visibility() const;
};

class LateCfsSchedule : public CfsSchedule {
  public:
    using CfsSchedule::CfsSchedule;
};

class RecoverySchedule : public BaseAtlasSchedule {
  public:
    using BaseAtlasSchedule::BaseAtlasSchedule;
    RecoverySchedule(unsigned id, AtlasJob *job, int submission_time, unsigned core,
                     int begin, unsigned execution_time)
        : BaseAtlasSchedule(id, job, submission_time, core, AtlasSchedulerType::recovery, begin,
                            execution_time) {}
    RecoverySchedule(AtlasJob *job, int submission_time, unsigned core, int begin,
                     unsigned execution_time)
        : BaseAtlasSchedule(job, submission_time, core, AtlasSchedulerType::recovery, begin,
                            execution_time) {}
    RecoverySchedule(const BaseAtlasSchedule *s);
};
