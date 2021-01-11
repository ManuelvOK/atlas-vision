#include <gui/job_arrow.h>

JobArrow::JobArrow(const Job *job, int pos_x, const InterfaceModel *interface_model,
                   SDL_GUI::Position position, Arrow::Direction direction):
    Arrow("JobArrow", position, direction),
    _job(job),
    _pos_x(pos_x),
    _interface_model(interface_model) {
    this->_default_style._color = this->_interface_model->get_color(this->_job->_id);
}

void JobArrow::update() {
    /* TODO: get rid of magic number */
    this->set_x(10 + this->_interface_model->px_width(this->_pos_x));
}
