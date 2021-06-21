#include <gui/budget_line.h>

BudgetLine::BudgetLine(InterfaceModel *interface_model, const PlayerModel *player_model,
                       int interval_begin, int interval_end, int y_begin, int y_end)
    : SDL_GUI::Line({0, y_begin}, {0, y_end}),
      _interface_model(interface_model),
      _player_model(player_model),
      _interval_begin(interval_begin),
      _interval_end(interval_end),
      _y_begin(y_begin),
      _y_end(y_end) {
}

SDL_GUI::Drawable *BudgetLine::clone() const {
    return new BudgetLine(*this);
}

void BudgetLine::update() {
    if (this->_interval_begin > this->_player_model->_position) {
        this->hide();
        return;
    }
    this->show();

    int line_begin_x = this->_interface_model->px_width(this->_interval_begin);
    int line_end_x = this->_interface_model->px_width(this->_interval_end);
    int player_position_x = this->_interface_model->px_width(this->_player_model->_position);
    int line_end_y = this->_y_end;

    /* clip at position line */
    if (this->_interval_end > this->_player_model->_position) {
        if (this->_y_begin != this->_y_end) {
            int distance_y = this->_y_end - this->_y_begin;
            int distance_x = line_end_x - line_begin_x;
            line_end_y = (player_position_x - line_begin_x) * 1.0 / distance_x * distance_y + this->_y_begin;
        }
        line_end_x = player_position_x;
    }

    this->set_begin({line_begin_x, this->_begin._y});
    this->set_end({line_end_x, line_end_y});
}
