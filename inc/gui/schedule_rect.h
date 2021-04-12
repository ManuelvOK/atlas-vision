#pragma once

#include <SDL_GUI/gui/primitives/rect.h>

#include <gui/job_highlight.h>
#include <models/interface_model.h>
#include <models/player_model.h>
#include <models/schedule.h>

class ScheduleRect: public SDL_GUI::Rect, public JobHighlight {
protected:
    const Schedule *_schedule;
    const InterfaceModel *_interface_model;
    const PlayerModel *_player_model;
    SDL_GUI::Style _default_style;
    virtual SDL_GUI::Drawable *clone() const override;
public:
    ScheduleRect(const Schedule *schedule, const InterfaceModel *interface_model,
                 const PlayerModel *player_model, const AtlasModel *atlas_model);

    void update() override;
};
