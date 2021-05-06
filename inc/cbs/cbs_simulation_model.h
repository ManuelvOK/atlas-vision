#pragma once

#include <cbs/cbs_job.h>
#include <cbs/cbs_schedule.h>
#include <cbs/constant_bandwidth_server.h>
#include <gui/job_arrow.h>
#include <simulation/simulation_model.h>


class CbsSimulationModel : public SimulationModel<CbsSchedule, CbsJob> {
  public:

    std::vector<HardRtJob *> _hard_rt_jobs;
    std::vector<SoftRtJob *> _soft_rt_jobs;

    std::vector<HardRtSchedule *> _hard_rt_schedules;

    std::set<HardRtJob *, decltype(&compare_jobs_deadline<HardRtJob>)> _hard_rt_queue;

    std::map<unsigned, ConstantBandwidthServer> _servers;

    CbsSchedule *_active_schedule = nullptr;

    std::map<SDL_GUI::Drawable *, JobArrow *> _arrows;
    std::vector<JobArrow *> _dl_arrows;

    std::map<const BaseJob *, SoftRtJob *> _soft_rt_job_mapping;


    CbsSimulationModel()
        : SimulationModel(), _hard_rt_queue(compare_jobs_deadline<HardRtJob>) {}

    void add_job(HardRtJob *job);
    void add_job(SoftRtJob *job);
    void add_schedule(HardRtSchedule *schedule);

    void enqueue_job(HardRtJob *job);
    void dequeue_job(HardRtJob *job);

    std::vector<BaseJob *> jobs() const;
    std::vector<BaseSchedule *> schedules() const;

    CbsJob *next_job() const;
    HardRtJob *next_hard_rt_job() const;
    SoftRtJob *next_soft_rt_job() const;

    CbsSchedule *active_schedule() const;
};
