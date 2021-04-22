#include <controllers/cbs_view_controller.h>

CbsViewController::CbsViewController(SDL_GUI::ApplicationBase *application,
                                     CbsModel *cbs_model,
                                     InterfaceModel *interface_model,
                                     SDL_GUI::InterfaceModel *default_interface_model,
                                     InputModel *input_model,
                                     const PlayerModel *player_model)
    : SimulationViewController(application, cbs_model, interface_model, default_interface_model,
                               input_model, player_model),
      _cbs_model(cbs_model) {}

void CbsViewController::create_schedule_drawables() {
}

void CbsViewController::init_cores_rect() {
}

SDL_GUI::Drawable *CbsViewController::create_job_information(const Job *job) {
    (void) job;
    return new SDL_GUI::NullDrawable;
}
