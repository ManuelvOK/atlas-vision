#pragma once

#include <SDL_GUI/inc/gui/primitives/line.h>

#include <models/cfs_visibility.h>
#include <models/interface_model.h>
#include <models/player_model.h>
#include <models/schedule.h>
/**
 * line that visualises the visibility of a schedule for the CFS scheduler.
 *
 * It is drawn from the current player position at the CFS scheduler to the beginning of the related
 * schedule
 */
class VisibilityLine : public SDL_GUI::Line {
    const InterfaceModel *_interface_model;
    const PlayerModel *_player_model;

    const CfsVisibility *_visibility; /**< related schedule */
    const Schedule *_schedule;

public:

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param schedule
     *   related schedule
     */
    VisibilityLine(InterfaceModel *interface_model, const PlayerModel *player_model, const CfsVisibility *visibility, const Schedule *schedule);

    void update() override;
};
