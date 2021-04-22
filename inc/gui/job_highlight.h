#pragma once

#include <SDL_GUI/gui/style.h>

#include <models/simulation_model.h>
#include <models/interface_model.h>
#include <models/job.h>

class JobHighlight {
private:
    const Job *_job;
    SDL_GUI::Style _highlight_style;
    const SimulationModel *_simulation_model;
protected:
    JobHighlight(const Job *job, const InterfaceModel *interface_model,
                 const SimulationModel *simulation_model);
    SDL_GUI::Style highlight_style();
    bool is_highlighted();
};
