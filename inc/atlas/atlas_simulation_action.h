#pragma once

#include <algorithm>
#include <string>
#include <vector>


#include <atlas/atlas_job.h>
#include <atlas/atlas_simulation_model.h>
#include <atlas/cfs_visibility.h>
#include <printable.h>
#include <schedule.h>
#include <simulation/core_assigner.h>
#include <simulation/simulation_action.h>

class WithAtlas : public WithModel<AtlasSimulationModel>,
                  public WithJob<AtlasJob> {
  public:
    WithAtlas(AtlasSimulationModel *atlas_model, AtlasJob *job)
        : WithModel(atlas_model),
          WithJob(job) {}
};

class AtlasSubmissionAction : public WithAtlas,
                              public SubmissionAction {
  public:
    AtlasSubmissionAction(CoreAssigner *core_assigner, AtlasSimulationModel *atlas_model,
                          AtlasJob* job)
        : WithAtlas(atlas_model, job),
          SubmissionAction(core_assigner, job, -50) {}

    void execute() override;
};

class AtlasDeadlineAction : public WithAtlas,
                            public DeadlineAction {
  public:
    AtlasDeadlineAction(AtlasSimulationModel *atlas_model, AtlasJob *job)
        : WithAtlas(atlas_model, job),
          DeadlineAction(job, -40) {}
    void execute() override;
};

class AtlasFillAction : public WithModel<AtlasSimulationModel>,
                        public TimedAction {
    unsigned _core;
  public:
    AtlasFillAction(AtlasSimulationModel *atlas_model, unsigned time, unsigned core)
        : WithModel(atlas_model),
          TimedAction(time, 60),
          _core(core) {}

    void execute() override;
};

template <typename T>
class AtlasBeginScheduleAction : public WithAtlas,
                                 public WithSchedule<T>,
                                 public SimulationAction {
    void end_schedule(BaseAtlasSchedule *schedule);
    void add_end_action() const;
  public:
    AtlasBeginScheduleAction(AtlasSimulationModel *atlas_model, AtlasJob *job, T *schedule)
        : WithAtlas(atlas_model, job),
          WithSchedule<T>(schedule),
          SimulationAction(50) {}

    virtual unsigned time() const override;
    virtual void execute() override;
};

template <typename T>
class AtlasEndScheduleAction : public WithAtlas,
                               public WithSchedule<T>,
                               public SimulationAction {
  public:
    AtlasEndScheduleAction(AtlasSimulationModel *atlas_model, AtlasJob *job, T *schedule)
        : WithAtlas(atlas_model, job),
          WithSchedule<T>(schedule),
          SimulationAction(40) {}

    virtual unsigned time() const override;
    virtual void execute() override;
};
