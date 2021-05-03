#include <gui/job_arrow.h>

JobArrow::JobArrow(const BaseJob *job, unsigned pos_x, const InterfaceModel *interface_model,
                   const BaseSimulationModel *simulation_model, SDL_GUI::Position position,
                   Arrow::Direction direction):
    Arrow("JobArrow", position, direction),
    JobHighlight(job, interface_model, simulation_model),
    _job(job),
    _pos_x(pos_x),
    _interface_model(interface_model) {
    this->_default_style._has_background = true;
    this->_default_style._color = this->_interface_model->get_color(this->_job->_id);
}

const BaseJob *JobArrow::job() const {
    return this->_job;
}

SDL_GUI::Drawable *JobArrow::clone() const {
    return new JobArrow(*this);
}

void JobArrow::set_pos_x(unsigned pos_x) {
    this->_pos_x = pos_x;
}

void JobArrow::update() {
    /* TODO: get rid of magic number */
    this->set_x(10 + this->_interface_model->px_width(this->_pos_x) - this->_width / 2);

    if (this->is_hidden()) {
        return;
    }
    this->_style = this->_default_style;
    if (this->is_highlighted()) {
        this->_style = this->highlight_style();
    }
}
