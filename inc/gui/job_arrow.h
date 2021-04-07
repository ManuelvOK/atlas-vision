#pragma once

#include <SDL_GUI/inc/gui/position.h>

#include <gui/arrow.h>
#include <gui/job_highlight.h>
#include <models/atlas_model.h>
#include <models/job.h>
#include <models/interface_model.h>

class JobArrow: public Arrow, public JobHighlight {
protected:
    const Job *_job;
    int _pos_x;
    const InterfaceModel *_interface_model;
    SDL_GUI::Style _default_style;

    virtual SDL_GUI::Drawable *clone() const override;
public:
    JobArrow(const Job *job, int pos_x, const InterfaceModel *interface_model,
             const AtlasModel *atlas_model, SDL_GUI::Position position,
             Arrow::Direction direction = Arrow::Direction::UP);

    void update() override;
};
