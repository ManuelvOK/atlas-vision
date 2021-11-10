#pragma once

#include <schedule.h>

struct GrubScheduleData : public ScheduleData {

};

class GrubJob;
class GrubSchedule : public Schedule<GrubScheduleData> {
  protected:
    GrubJob *_grub_job;
  public:
    GrubSchedule(unsigned id, GrubJob *job, int submission_time, unsigned core, int begin,
                 unsigned execution_time);
    GrubSchedule(GrubJob *job, int submission_time, unsigned core, int begin,
                 unsigned execution_time);

    GrubJob *grub_job() const;

    std::string to_string() const override;

};

class HardGrubJob;
class HardGrubSchedule : public GrubSchedule {
  public:
    HardGrubJob *_grub_job;

    HardGrubSchedule(HardGrubJob *job, int submission_time, unsigned core, int begin,
                     unsigned execution_time);
};

class SoftGrubJob;
class SoftGrubSchedule : public GrubSchedule {
  public:
    SoftGrubJob *_grub_job;

    SoftGrubSchedule(SoftGrubJob *job, int submission_time, unsigned core, int begin,
                     unsigned execution_time);

    GuiScheduleData get_vision_data_at_time(int timestamp) const override;
};
