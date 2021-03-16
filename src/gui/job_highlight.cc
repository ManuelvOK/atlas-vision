#include <gui/job_highlight.h>


JobHighlight::JobHighlight(const Job *job, const InterfaceModel *interface_model,
                           const AtlasModel *atlas_model) :
    _job(job),
    _atlas_model(atlas_model) {
    this->_style._color = interface_model->get_color(this->_job->_id);
    this->_style._has_background = true;
    this->_style._has_border = true;
    this->_style._border_color = SDL_GUI::RGB("white");
}

SDL_GUI::Style *JobHighlight::highlight_style() {
    return &this->_style;
}

bool JobHighlight::is_highlighted() {
    if (this->_atlas_model->_highlighted_jobs.contains(this->_job->_id)) {
        return true;
    }
    return false;
}
