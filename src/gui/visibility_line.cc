#include <gui/visibility_line.h>

#include <config/interface_config.h>

VisibilityLine::VisibilityLine(InterfaceModel *interface_model, const PlayerModel *player_model,
                               const CfsVisibility *visibility, const Schedule *schedule)
    : SDL_GUI::Line(), _interface_model(interface_model), _player_model(player_model),
    _visibility(visibility), _schedule(schedule) {
    int begin_y = interface_config.schedule.CFS_offset_y + 0.5
                  * this->_interface_model->px_height(1);
    this->set_position({0, begin_y});
    int end_y = interface_config.schedule.ATLAS_offset_y - begin_y + 0.5
                * this->_interface_model->px_height(1);
    this->_end.set_position({0, end_y});
}

void VisibilityLine::update() {
    /* hide if not active */
    if (this->_visibility->is_active_at_time(this->_player_model->_position)) {
        this->show();
    } else {
        this->hide();
        return;
    }

    /* set begin and end */
    int line_begin_x = this->_interface_model->px_width(this->_player_model->_position);
    this->set_x(line_begin_x + 1);
    int schedule_begin_x =
        std::get<0>(this->_schedule->get_data_at_time(this->_player_model->_position));
    this->_end.set_x(this->_interface_model->px_width(schedule_begin_x
                                                      - this->_player_model->_position) - 1);
}
