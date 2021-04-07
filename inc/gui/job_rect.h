#pragma once

#include <SDL_GUI/inc/gui/primitives/rect.h>

#include <models/atlas_model.h>
#include <models/interface_model.h>
#include <models/job.h>
#include <gui/job_highlight.h>

class JobRect: public SDL_GUI::Rect, public JobHighlight {
protected:
    const Job *_job;
    const InterfaceModel *_interface_model;
    SDL_GUI::Style _default_style;
    virtual SDL_GUI::Drawable *clone() const override;
public:
    JobRect(const Job *job, const InterfaceModel *interface_model, const AtlasModel *atlas_model,
            SDL_GUI::Position position, int width, int height);

    void update() override;
};
