#include <gui/job_rect.h>


JobRect::JobRect(const Job *job, const InterfaceModel *interface_model,
                 const AtlasModel *atlas_model, SDL_GUI::Position position, int width, int height) :
    SDL_GUI::Rect("JobRect", position, width, height),
    JobHighlight(job, interface_model, atlas_model),
    _job(job),
    _interface_model(interface_model) {
    this->_default_style._color = this->_interface_model->get_color(this->_job->_id);
    this->_default_style._has_background = true;
    this->_default_style._has_border = true;
}

SDL_GUI::Drawable *JobRect::clone() const {
    return new JobRect(*this);
}

void JobRect::update() {
    this->set_current_style(&this->_default_style);
    if (this->is_hidden()) {
        return;
    }
    if (this->is_highlighted()) {
        this->set_current_style(this->highlight_style());
    }
}
