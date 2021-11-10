#pragma once

#include <grub/grub_schedule.h>
#include <job.h>

class GrubJob: public Job<GrubSchedule> {
    using Job<GrubSchedule>::Job;
  public:
    virtual ~GrubJob() = default;
};

class HardGrubJob: public GrubJob {
  public:
    typedef HardGrubSchedule ScheduleType;
    using GrubJob::GrubJob;

    std::string to_string() const override;
};

class GrubConstantBandwidthServer;
class SoftGrubJob: public GrubJob {
    std::map<int, int> _deadlines;
  public:
    typedef SoftGrubSchedule ScheduleType;

    GrubConstantBandwidthServer *_cbs = nullptr;

    SoftGrubJob(unsigned id, unsigned execution_time, int submission_time)
        : GrubJob(id, 0, execution_time, submission_time) {}

    int deadline(int timestamp) const;

    void add_change_deadline(int timestamp, int deadline);

    std::string to_string() const override;
};
