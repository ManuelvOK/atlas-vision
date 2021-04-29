#pragma once

#include <SDL_GUI/models/model_base.h>

#include <simulation/core_assigner.h>
#include <job.h>
#include <schedule.h>

template <typename T>
class WithModel {
    static_assert(std::is_base_of<SDL_GUI::ModelBase, T>::value);
  public:
    T *_model;
    WithModel(T *model)
        : _model(model) {}
};

template <typename T>
class WithJob {
    static_assert(std::is_base_of<BaseJob, T>::value);
  public:
    T *_job;
    WithJob(T *job)
        : _job(job) {}
};

template <typename T>
class WithSchedule {
    static_assert(std::is_base_of<BaseSchedule, T>::value);
  protected:
    T *_schedule;
  public:
    WithSchedule(T *schedule)
        : _schedule(schedule) {}
};

class SimulationAction {
  public:
    int _weight = 0;
    bool _success = true;
    SimulationAction(int weight = 0)
        : _weight(weight) {}
    virtual ~SimulationAction() = default;
    virtual unsigned time() const = 0;
    virtual void execute() = 0;
};

class TimedAction : public SimulationAction {
  public:
    unsigned _time;
    TimedAction(unsigned time, int weight = 0)
        : SimulationAction(weight), _time(time) {}

    virtual unsigned time() const override;
};

class SubmissionAction : public TimedAction {
  protected:
    CoreAssigner *_core_assigner;
  public:
    SubmissionAction(CoreAssigner *core_assigner, BaseJob *job, int weight = 0)
        : TimedAction(job->_submission_time, weight),
          _core_assigner(core_assigner) {}
};

class DeadlineAction : public TimedAction {
  public:
    DeadlineAction(BaseJob *job, int weight = 0)
        : TimedAction(job->_deadline, weight) {}
};

