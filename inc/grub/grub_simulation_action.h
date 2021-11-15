#pragma once

#include <simulation/simulation_action.h>
#include <grub/grub_simulation_model.h>

class WithGrub : public WithModel<GrubSimulationModel>,
                 public WithJob<GrubJob> {
  public:
    WithGrub(GrubSimulationModel *grub_model, GrubJob *job)
        : WithModel(grub_model),
          WithJob(job) {}
};

template <typename T>
class GrubSubmissionAction : public WithModel<GrubSimulationModel>,
                            public WithJob<T>,
                            public SubmissionAction {
    static_assert(std::is_base_of<GrubJob, T>::value);
  public:
    GrubSubmissionAction(CoreAssigner *core_assigner, GrubSimulationModel *grub_model, T *job)
        : WithModel(grub_model),
          WithJob<T>(job),
          SubmissionAction(core_assigner, job, -50) {}

    void execute() override;
};

class GrubFillAction : public WithModel<GrubSimulationModel>,
                      public TimedAction {

  public:
    GrubFillAction(GrubSimulationModel *grub_model, int time)
        : WithModel(grub_model),
          TimedAction(time, 60) {}
    void execute() override;
};

template <typename T>
class GrubBeginScheduleAction :  public WithGrub,
                                 public WithSchedule<T>,
                                 public SimulationAction {
    static_assert(std::is_base_of<GrubSchedule, T>::value);
  public:
    GrubBeginScheduleAction(GrubSimulationModel *grub_model, GrubJob *job, T *schedule)
        : WithGrub(grub_model, job),
          WithSchedule<T>(schedule),
          SimulationAction(50) {}

    virtual int time() const override;
    virtual void execute() override;
};

class GrubPostponeDeadlineAction : public WithModel<GrubSimulationModel>,
                                   public SimulationAction {
    GrubConstantBandwidthServer *_server;
  public:
    GrubPostponeDeadlineAction(GrubSimulationModel *grub_model, GrubConstantBandwidthServer *grub_server)
        : WithModel<GrubSimulationModel>(grub_model),
          SimulationAction(0),
          _server(grub_server) {}

    virtual int time() const override;
    virtual void execute() override;
};

template <typename T>
class GrubEndScheduleAction :  public WithGrub,
                              public WithSchedule<T>,
                              public SimulationAction {
    static_assert(std::is_base_of<GrubSchedule, T>::value);
  public:
    GrubEndScheduleAction(GrubSimulationModel *grub_model, GrubJob *job, T *schedule)
        : WithGrub(grub_model, job),
          WithSchedule<T>(schedule),
          SimulationAction(40) {}

    virtual int time() const override;
    virtual void execute() override;
};

class GrubDeactivateServerAction : public WithModel<GrubSimulationModel>,
                                   public TimedAction {
    GrubConstantBandwidthServer *_server;
  public:
    GrubDeactivateServerAction(GrubSimulationModel *grub_model, int deactivation_time, GrubConstantBandwidthServer *grub_server)
        : WithModel<GrubSimulationModel>(grub_model),
          TimedAction(deactivation_time, 0),
          _server(grub_server) {}

    virtual int time() const override;
    virtual void execute() override;
};
