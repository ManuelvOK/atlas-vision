#pragma once

#include <SDL_GUI/gui/position.h>

#include <gui/arrow.h>
#include <gui/interface_model.h>
#include <gui/job_highlight.h>
#include <job.h>
#include <simulation/simulation_model.h>

class JobArrow: public Arrow, public JobHighlight {
protected:
    const BaseJob *_job;
    int _pos_x;
    const InterfaceModel *_interface_model;
    SDL_GUI::Style _default_style;

    virtual SDL_GUI::Drawable *clone() const override;
public:
    JobArrow(const BaseJob *job, int pos_x, const InterfaceModel *interface_model,
             const BaseSimulationModel *simulation_model, SDL_GUI::Position position,
             Arrow::Direction direction = Arrow::Direction::UP);

    void update() override;
};
