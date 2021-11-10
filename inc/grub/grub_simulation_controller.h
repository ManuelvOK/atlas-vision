#pragma once

#include <grub/grub_simulation_model.h>
#include <simulation/simulation_controller.h>

class GrubSimulationController : public SimulationController<GrubSchedule, GrubJob> {
    GrubSimulationModel *_grub_model;

    void bootstrap_simulation() override;
    void write_back(std::string output_file) const override;
    std::vector<std::string> check_simulation() const override;
  public:
    GrubSimulationController(SDL_GUI::ApplicationBase *application,
                             GrubSimulationModel *grub_model,
                             PlayerModel *player_model);
};

