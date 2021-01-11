#pragma once

#include <SDL_GUI/inc/gui/style.h>

#include <models/atlas_model.h>
#include <models/interface_model.h>
#include <models/job.h>

class JobHighlight {
private:
    const Job *_job;
    SDL_GUI::Style _style;
    const AtlasModel *_atlas_model;
protected:
    JobHighlight(const Job *job, const InterfaceModel *interface_model,
                 const AtlasModel *atlas_model);
    SDL_GUI::Style *highlight_style();
    bool is_highlighted();
};
