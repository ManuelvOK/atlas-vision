#pragma once

#include <SDL_GUI/gui/style.h>

#include <gui/interface_model.h>
#include <job.h>
#include <simulation/simulation_model.h>

class JobHighlight {
private:
    const BaseJob *_job;
    SDL_GUI::Style _highlight_style;
    const BaseSimulationModel *_simulation_model;
protected:
    JobHighlight(const BaseJob *job, const InterfaceModel *interface_model,
                 const BaseSimulationModel *simulation_model);
    SDL_GUI::Style highlight_style();
    bool is_highlighted();
};
