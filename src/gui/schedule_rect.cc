#include <gui/schedule_rect.h>

ScheduleRect::ScheduleRect(const Schedule *schedule, const InterfaceModel *interface_model,
                           const PlayerModel *player_model, std::map<SchedulerType, int> offsets):
    SDL_GUI::Rect("ScheduleRect"),
    _schedule(schedule),
    _interface_model(interface_model),
    _player_model(player_model),
    _offsets(offsets) {
    this->set_height(this->_interface_model->px_height(1));
    this->_default_style._color = this->_interface_model->get_color(this->_schedule->_job_id);
    this->_default_style._has_background = true;
    this->_default_style._has_border = true;
}

void ScheduleRect::update() {
    int timestamp = this->_player_model->_position;
    if (not this->_schedule->exists_at_time(timestamp)) {
        this->hide();
        return;
    }
    this->show();

    int begin;
    SchedulerType scheduler;
    int execution_time;
    std::tie(begin, scheduler, execution_time) = this->_schedule->get_data_at_time(timestamp);

    this->set_x(this->_interface_model->px_width(begin));
    this->set_y(this->_offsets.at(scheduler));
    this->set_width(this->_interface_model->px_width(execution_time));
}
