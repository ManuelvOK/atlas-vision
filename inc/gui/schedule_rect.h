#pragma once

#include <SDL_GUI/gui/primitives/rect.h>

#include <gui/interface_model.h>
#include <gui/job_highlight.h>
#include <player/player_model.h>
#include <schedule.h>

class ScheduleRect: public SDL_GUI::Rect, public JobHighlight {
protected:
    const BaseSchedule *_schedule;
    const InterfaceModel *_interface_model;
    const PlayerModel *_player_model;
    SDL_GUI::Style _default_style;
    virtual SDL_GUI::Drawable *clone() const override;
public:
    ScheduleRect(const BaseSchedule *schedule, const InterfaceModel *interface_model,
                 const PlayerModel *player_model, const BaseSimulationModel *simulation_model);

    virtual void update() override;
};
