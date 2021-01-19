#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <models/cfs_visibility.h>
#include <models/printable.h>
#include <models/schedule.h>
#include <models/job.h>

class AtlasModel;

class SimulationAction {
  protected:
    AtlasModel *_atlas_model;
  public:
    SimulationAction(AtlasModel *atlas_model) : _atlas_model(atlas_model) {}
    virtual ~SimulationAction() = default;
    virtual int time() const = 0;
    virtual void action() = 0;
};

class TimedAction : public SimulationAction {
  public:
    int _time;
    TimedAction(AtlasModel *atlas_model,int time)
        : SimulationAction(atlas_model), _time(time) {}
    virtual int time() const override;
};

class SubmissionAction : public TimedAction {
    std::vector<Job *> _jobs;
  public:
    SubmissionAction(AtlasModel *atlas_model, int time, std::vector<Job*> jobs)
        : TimedAction(atlas_model, time), _jobs(jobs) {}
    virtual void action() override;
};

class DeadlineAction : public TimedAction {
    Job *_job;
  public:
    DeadlineAction(AtlasModel *atlas_model, int time, Job *job)
        : TimedAction(atlas_model, time), _job(job) {}
    virtual void action() override;
};

class FillAction : public TimedAction {
  public:
    FillAction(AtlasModel *atlas_model, int time)
        : TimedAction(atlas_model, time) {}

    virtual void action() override;
};

template <typename T>
class BeginScheduleAction : public SimulationAction {
    void endSchedule(Schedule *schedule);
    void add_end_action() const;
  protected:
    T *_schedule;
  public:
    BeginScheduleAction(AtlasModel *atlas_model, T *schedule)
        : SimulationAction(atlas_model), _schedule(schedule) {}

    virtual int time() const override;
    virtual void action() override;
};

template <typename T>
class EndScheduleAction : public SimulationAction {
  protected:
    T *_schedule;
  public:
    EndScheduleAction(AtlasModel *atlas_model, T *schedule)
        : SimulationAction(atlas_model), _schedule(schedule) {}
//    EndScheduleAction(AtlasModel *atlas_model, Schedule *schedule)
//        : Action(atlas_model), schedule(schedule) {}
    virtual int time() const override;
    virtual void action() override;
};
