#include <gui/job_arrow.h>

JobArrow::JobArrow(const Job *job, int pos_x, const InterfaceModel *interface_model,
                   const AtlasModel *atlas_model, SDL_GUI::Position position,
                   Arrow::Direction direction):
    Arrow("JobArrow", position, direction),
    JobHighlight(job, interface_model, atlas_model),
    _job(job),
    _pos_x(pos_x),
    _interface_model(interface_model) {
    this->_default_style._color = this->_interface_model->get_color(this->_job->_id);
}

SDL_GUI::Drawable *JobArrow::clone() const {
    return new JobArrow(*this);
}

void JobArrow::update() {
    /* TODO: get rid of magic number */
    this->set_x(10 + this->_interface_model->px_width(this->_pos_x) - this->_width / 2);

    this->set_current_style(&this->_default_style);
    if (this->is_hidden()) {
        return;
    }
    if (this->is_highlighted()) {
        this->set_current_style(this->highlight_style());
    }
}
