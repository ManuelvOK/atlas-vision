#pragma once

#include <gui/job_arrow.h>
#include <grub/grub_job.h>
#include <grub/grub_schedule.h>
#include <grub/grub_constant_bandwidth_server.h>
#include <simulation/simulation_model.h>

class GrubSimulationModel : public SimulationModel<GrubSchedule, GrubJob> {
    float _total_utilisation = 0;
  public:
    std::vector<HardGrubJob *> _hard_jobs;
    std::vector<SoftGrubJob *> _soft_jobs;

    std::vector<HardGrubSchedule *> _hard_schedules;

    std::set<HardGrubJob *, decltype(&compare_jobs_deadline<HardGrubJob>)> _hard_queue;

    std::map<unsigned, GrubConstantBandwidthServer> _servers;

    std::map<int, GrubSchedule *> _active_schedules;

    std::map<SDL_GUI::Drawable *, JobArrow *> _arrows;
    std::vector<JobArrow *> _dl_arrows;

    std::map<const BaseJob *, SoftGrubJob *> _soft_job_mapping;

    GrubSimulationModel()
        : SimulationModel(), _hard_queue(compare_jobs_deadline<HardGrubJob>) {}

    float total_utilisation() const;
    void change_total_utilisation(float difference);

    void add_job(HardGrubJob *job);
    void add_job(SoftGrubJob *job);
    void add_schedule(HardGrubSchedule *schedule);

    void enqueue_job(HardGrubJob *job);
    void dequeue_job(HardGrubJob *job);

    void update_servers_virtual_times();

    std::vector<BaseJob *> jobs() const override;
    std::vector<BaseSchedule *> schedules() const override;

    GrubJob *next_job(unsigned skip = 0) const;
    HardGrubJob *next_hard_job(unsigned skip = 0) const;
    SoftGrubJob *next_soft_job(unsigned skip = 0) const;

    GrubSchedule *active_schedule(unsigned core) const;
};
