#pragma once

#include <cbs/cbs_job.h>
#include <cbs/cbs_simulation_model.h>
#include <simulation/core_assigner.h>
#include <simulation/simulation_action.h>


class WithCbs : public WithModel<CbsSimulationModel>,
                public WithJob<CbsJob> {
  public:
    WithCbs(CbsSimulationModel *cbs_model, CbsJob *job)
        : WithModel(cbs_model),
          WithJob(job) {}
};

template <typename T>
class CbsSubmissionAction : public WithModel<CbsSimulationModel>,
                            public WithJob<T>,
                            public SubmissionAction {
    static_assert(std::is_base_of<CbsJob, T>::value);
  public:
    CbsSubmissionAction(CoreAssigner *core_assigner, CbsSimulationModel *cbs_model, T *job)
        : WithModel(cbs_model),
          WithJob<T>(job),
          SubmissionAction(core_assigner, job, -50) {}

    void execute() override;
};

class CbsDeadlineAction : public WithCbs,
                          public DeadlineAction {
  public:
    CbsDeadlineAction(CbsSimulationModel *cbs_model, CbsJob *job)
        : WithCbs(cbs_model, job),
          DeadlineAction(job, -40) {}
    void execute() override;
};

class CbsFillAction : public WithModel<CbsSimulationModel>,
                      public TimedAction {
    unsigned _core;
  public:
    CbsFillAction(CbsSimulationModel *cbs_model, int time, unsigned core)
        : WithModel(cbs_model),
          TimedAction(time, 60),
          _core(core) {}
    void execute() override;
};

template <typename T>
class CbsBeginScheduleAction :  public WithCbs,
                                public WithSchedule<T>,
                                public SimulationAction {
    static_assert(std::is_base_of<CbsSchedule, T>::value);
  public:
    CbsBeginScheduleAction(CbsSimulationModel *cbs_model, CbsJob *job, T *schedule)
        : WithCbs(cbs_model, job),
          WithSchedule<T>(schedule),
          SimulationAction(50) {}

    virtual int time() const override;
    virtual void execute() override;
};

template <typename T>
class CbsEndScheduleAction :  public WithCbs,
                              public WithSchedule<T>,
                              public SimulationAction {
    static_assert(std::is_base_of<CbsSchedule, T>::value);
  public:
    CbsEndScheduleAction(CbsSimulationModel *cbs_model, CbsJob *job, T *schedule)
        : WithCbs(cbs_model, job),
          WithSchedule<T>(schedule),
          SimulationAction(40) {}

    virtual int time() const override;
    virtual void execute() override;
};
