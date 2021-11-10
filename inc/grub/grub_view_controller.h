#pragma once

#include <grub/grub_simulation_model.h>
#include <simulation/simulation_view_controller.h>

class GrubViewController : public SimulationViewController<GrubSchedule, GrubJob> {

    GrubSimulationModel *_grub_model;

    void init() override;

    void create_schedule_drawables() override;

    void init_cores_rect() override;

    virtual void create_legend(std::vector<GrubJob *> jobs) override;

    SDL_GUI::Drawable *create_job_information(const GrubJob *job) override;

    virtual std::vector<JobArrow *> create_submission_drawables(std::map<int, std::vector<unsigned>> deadlines) override;

    virtual std::vector<JobArrow *> create_deadline_drawables(std::map<int, std::vector<unsigned>> deadlines) override;

public:
    GrubViewController(SDL_GUI::ApplicationBase *application,
                       GrubSimulationModel *grub_model,
                       InterfaceModel *interface_model,
                       SDL_GUI::InterfaceModel *default_interface_model,
                       InputModel *input_model,
                       const PlayerModel *player_model);

    virtual void update() override;
};

