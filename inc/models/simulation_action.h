#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <controllers/core_assigner.h>
#include <models/cfs_visibility.h>
#include <models/printable.h>
#include <models/schedule.h>
#include <models/job.h>

class AtlasModel;

class SimulationAction {
  protected:
    AtlasModel *_atlas_model;
  public:
    int _weight = 0;
    SimulationAction(AtlasModel *atlas_model, int weight = 0);
    virtual ~SimulationAction() = default;
    virtual int time() const = 0;
    virtual void action() = 0;
};

class TimedAction : public SimulationAction {
  public:
    int _time;
    TimedAction(AtlasModel *atlas_model,int time, int weight = 0)
        : SimulationAction(atlas_model, weight), _time(time) {}
    virtual int time() const override;
};

class SubmissionAction : public TimedAction {
    CoreAssigner *_core_assigner;
    Job * _job;
  public:
    SubmissionAction(CoreAssigner *core_assigner, AtlasModel *atlas_model, Job* job) :
        TimedAction(atlas_model, job->_submission_time, -50),
        _core_assigner(core_assigner),
        _job(job) {}
    virtual void action() override;
};

class DeadlineAction : public TimedAction {
    Job *_job;
  public:
    DeadlineAction(AtlasModel *atlas_model, int time, Job *job)
        : TimedAction(atlas_model, time, -40), _job(job) {}
    virtual void action() override;
};

class FillAction : public TimedAction {
    unsigned _core;
  public:
    FillAction(AtlasModel *atlas_model, int time, unsigned core)
        : TimedAction(atlas_model, time), _core(core) {}

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
        : SimulationAction(atlas_model, 50), _schedule(schedule) {}

    virtual int time() const override;
    virtual void action() override;
};

template <typename T>
class EndScheduleAction : public SimulationAction {
  protected:
    T *_schedule;
  public:
    EndScheduleAction(AtlasModel *atlas_model, T *schedule)
        : SimulationAction(atlas_model, 40), _schedule(schedule) {}
//    EndScheduleAction(AtlasModel *atlas_model, Schedule *schedule)
//        : Action(atlas_model), schedule(schedule) {}
    virtual int time() const override;
    virtual void action() override;
};
