#include <gui/schedule_rect.h>

ScheduleRect::ScheduleRect(const Schedule *schedule, const InterfaceModel *interface_model,
                           const PlayerModel *player_model, const AtlasModel *atlas_model):
    SDL_GUI::Rect("ScheduleRect"),
    JobHighlight(schedule->_job, interface_model, atlas_model),
    _schedule(schedule),
    _interface_model(interface_model),
    _player_model(player_model) {
    this->set_height(this->_interface_model->px_height(1));
    this->_default_style._color = this->_interface_model->get_color(this->_schedule->_job->_id);
    this->_default_style._has_background = true;
    this->_default_style._has_border = true;
}

SDL_GUI::Drawable *ScheduleRect::clone() const {
    return new ScheduleRect(*this);
}

void ScheduleRect::update() {
    int timestamp = this->_player_model->_position;
    if (not this->_schedule->exists_at_time(timestamp)) {
        this->hide();
        return;
    }
    this->show();

    ScheduleData data = this->_schedule->get_vision_data_at_time(timestamp);

    this->set_x(this->_interface_model->px_width(data._begin));
    this->set_y(this->_interface_model->scheduler_offset(data._scheduler));
    this->set_width(this->_interface_model->px_width(data._execution_time));
    this->set_current_style(&this->_default_style);
    if (this->is_hidden()) {
        return;
    }
    if (this->is_highlighted()) {
        this->set_current_style(this->highlight_style());
    }
}
