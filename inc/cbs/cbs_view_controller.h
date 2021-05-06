#pragma once

#include <SDL_GUI/application.h>
#include <SDL_GUI/models/interface_model.h>

#include <cbs/cbs_simulation_model.h>
#include <gui/interface_model.h>
#include <input/input_model.h>
#include <player/player_model.h>
#include <simulation/simulation_view_controller.h>

class CbsViewController : public SimulationViewController<CbsSchedule, CbsJob> {

    CbsSimulationModel *_cbs_model;

    void init() override;

    void create_schedule_drawables() override;

    void init_cores_rect() override;

    virtual void create_legend(std::vector<CbsJob *> jobs) override;

    SDL_GUI::Drawable *create_job_information(const CbsJob *job) override;

    virtual std::vector<JobArrow *> create_submission_drawables(std::map<unsigned, std::vector<unsigned>> deadlines) override;

    virtual std::vector<JobArrow *> create_deadline_drawables(std::map<unsigned, std::vector<unsigned>> deadlines) override;

    void create_budget_lines(const std::map<unsigned, ConstantBandwidthServer> &servers);

    void create_budget_line(const ConstantBandwidthServer &cbs);

public:
    CbsViewController(SDL_GUI::ApplicationBase *application,
                      CbsSimulationModel *cbs_model,
                      InterfaceModel *interface_model,
                      SDL_GUI::InterfaceModel *default_interface_model,
                      InputModel *input_model,
                      const PlayerModel *player_model);

    virtual void update() override;
};
