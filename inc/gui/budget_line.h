#pragma once

#include <SDL_GUI/gui/primitives/line.h>

#include <gui/interface_model.h>
#include <player/player_model.h>

class BudgetLine : public SDL_GUI::Line {
    const InterfaceModel *_interface_model;
    const PlayerModel *_player_model;

    unsigned _interval_begin;
    unsigned _interval_end;

    int _y_begin;
    int _y_end;

    virtual SDL_GUI::Drawable *clone() const override;
  public:
    BudgetLine(InterfaceModel *interface_model, const PlayerModel *player_model,
               unsigned interval_begin, unsigned interval_end, int y_begin, int y_end);

    void update() override;
};
