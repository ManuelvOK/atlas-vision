#pragma once

#include <SDL_GUI/gui/primitives/rect.h>

#include <gui/interface_model.h>
#include <gui/job_highlight.h>
#include <job.h>
#include <simulation/simulation_model.h>

class JobRect: public SDL_GUI::Rect, public JobHighlight {
protected:
    const BaseJob *_job;
    const InterfaceModel *_interface_model;
    SDL_GUI::Style _default_style;
    virtual SDL_GUI::Drawable *clone() const override;
public:
    JobRect(const BaseJob *job, const InterfaceModel *interface_model,
            const BaseSimulationModel *atlas_model, SDL_GUI::Position position,
            int width, int height);

    void update() override;
};
