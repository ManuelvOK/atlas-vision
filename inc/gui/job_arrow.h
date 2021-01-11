#pragma once

#include <SDL_GUI/inc/gui/position.h>

#include <gui/arrow.h>
#include <models/job.h>
#include <models/interface_model.h>

class JobArrow: public Arrow {
protected:
    const Job *_job;
    int _pos_x;
    const InterfaceModel *_interface_model;

public:
    JobArrow(const Job *job, int pos_x, const InterfaceModel *interface_model,
             SDL_GUI::Position position, Arrow::Direction direction = Arrow::Direction::UP);

    void update() override;
};
