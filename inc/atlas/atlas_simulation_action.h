#pragma once

#include <algorithm>
#include <string>
#include <vector>


#include <atlas/atlas_job.h>
#include <atlas/cfs_visibility.h>
#include <printable.h>
#include <schedule.h>
#include <simulation/core_assigner.h>
#include <simulation/simulation_action.h>

class AtlasSimulationModel;

class AtlasSimulationAction : public SimulationAction {
  protected:
    AtlasSimulationModel *_atlas_model;
  public:
    AtlasSimulationAction(AtlasSimulationModel *atlas_model, int weight = 0)
        : SimulationAction(weight), _atlas_model(atlas_model) {}
};

class TimedAction : public AtlasSimulationAction {
  public:
    int _time;
    TimedAction(AtlasSimulationModel *atlas_model,int time, int weight = 0)
        : AtlasSimulationAction(atlas_model, weight), _time(time) {}
    virtual int time() const override;
};

class SubmissionAction : public TimedAction {
    CoreAssigner *_core_assigner;
    AtlasJob * _job;
  public:
    SubmissionAction(CoreAssigner *core_assigner, AtlasSimulationModel *atlas_model,
                     AtlasJob* job)
        : TimedAction(atlas_model, job->_submission_time, -50),
          _core_assigner(core_assigner),
          _job(job) {}
    virtual void execute() override;
};

class DeadlineAction : public TimedAction {
    AtlasJob *_job;
  public:
    DeadlineAction(AtlasSimulationModel *atlas_model, int time, AtlasJob *job)
        : TimedAction(atlas_model, time, -40), _job(job) {}
    virtual void execute() override;
};

class FillAction : public TimedAction {
    unsigned _core;
  public:
    FillAction(AtlasSimulationModel *atlas_model, int time, unsigned core)
        : TimedAction(atlas_model, time, 60), _core(core) {}

    virtual void execute() override;
};

template <typename T>
class BeginScheduleAction : public AtlasSimulationAction {
    void end_schedule(BaseAtlasSchedule *schedule);
    void add_end_action() const;
  protected:
    T *_schedule;
  public:
    BeginScheduleAction(AtlasSimulationModel *atlas_model, T *schedule)
        : AtlasSimulationAction(atlas_model, 50), _schedule(schedule) {}

    virtual int time() const override;
    virtual void execute() override;
};

template <typename T>
class EndScheduleAction : public AtlasSimulationAction {
  protected:
    T *_schedule;
  public:
    EndScheduleAction(AtlasSimulationModel *atlas_model, T *schedule)
        : AtlasSimulationAction(atlas_model, 40), _schedule(schedule) {}
//    EndScheduleAction(AtlasSimulationModel *atlas_model, BaseAtlasSchedule *schedule)
//        : Action(atlas_model), schedule(schedule) {}
    virtual int time() const override;
    virtual void execute() override;
};
