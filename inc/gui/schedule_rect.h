#pragma once

#include <SDL_GUI/inc/gui/primitives/rect.h>

#include <models/interface_model.h>
#include <models/player_model.h>
#include <models/schedule.h>

class ScheduleRect: public SDL_GUI::Rect {
protected:
    const Schedule *_schedule;
    const InterfaceModel *_interface_model;
    const PlayerModel *_player_model;
    const std::map<SchedulerType, int> _offsets;
public:
    ScheduleRect(const Schedule *schedule, const InterfaceModel *interface_model,
                 const PlayerModel *player_model, const std::map<SchedulerType, int> offsets);

    void update() override;
};
