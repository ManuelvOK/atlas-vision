#include <gui/visibility_line.h>

#include <config/interface_config.h>

VisibilityLine::VisibilityLine(InterfaceModel *interface_model, const PlayerModel *player_model,
                               const CfsVisibility visibility, const Schedule *schedule)
    : SDL_GUI::Line(), _interface_model(interface_model), _player_model(player_model),
    _visibility(visibility), _schedule(schedule) {
    int begin_y = this->_interface_model->scheduler_offset(SchedulerType::CFS)
                  + 0.5 * this->_interface_model->px_height(1);
    this->set_begin({0, begin_y});
    int end_y = this->_interface_model->scheduler_offset(SchedulerType::ATLAS)
                + 0.5 * this->_interface_model->px_height(1);
    this->set_end({0, end_y});
}

SDL_GUI::Drawable *VisibilityLine::clone() const {
    return new VisibilityLine(*this);
}

void VisibilityLine::update() {
    /* hide if not active */
    if (this->_visibility.is_active_at_time(this->_player_model->_position)) {
        this->show();
    } else {
        this->hide();
        return;
    }

    /* set begin and end */
    int line_begin_x = this->_interface_model->px_width(this->_player_model->_position);
    this->set_begin({line_begin_x + 1, this->_begin._y});
    ScheduleData data = this->_schedule->get_data_at_time(this->_player_model->_position);
    this->set_end({this->_interface_model->px_width(data._begin) - 1, this->_end._y});
}
