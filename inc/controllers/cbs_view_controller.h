#pragma once

#include <SDL_GUI/application.h>
#include <SDL_GUI/models/interface_model.h>

#include <controllers/simulation_view_controller.h>
#include <models/cbs_model.h>
#include <models/interface_model.h>
#include <models/input_model.h>
#include <models/player_model.h>

class CbsViewController : public SimulationViewController {

    CbsModel *_cbs_model;

    void create_schedule_drawables() override;

    void init_cores_rect() override;

    SDL_GUI::Drawable *create_job_information(const Job *job) override;

public:
    CbsViewController(SDL_GUI::ApplicationBase *application,
                      CbsModel *cbs_model,
                      InterfaceModel *interface_model,
                      SDL_GUI::InterfaceModel *default_interface_model,
                      InputModel *input_model,
                      const PlayerModel *player_model);
};
