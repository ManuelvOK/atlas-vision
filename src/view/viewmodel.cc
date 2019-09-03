#include <view/viewmodel.h>

#include <iostream>
#include <algorithm>

Viewmodel::Viewmodel(const Model *model) :
    config(), EDF_sorted_jobs(), schedules(), deadlines(), submissions(), colors() {
    float player_width_px = this->get_player_width_px();
    this->unit_w_min = player_width_px / model->_player._max_position;
    this->unit_w = this->unit_w_min;
    this->unit_h = this->config.unit.height_px;
    this->n_jobs = model->_jobs.size();
    this->jobs = &model->_jobs;
    this->n_schedules = model->_schedules.size();
    this->init_colors();
    this->init_EDF_sorted_jobs(model);
    this->init_submissions(model);
    this->init_deadlines(model);
    this->init_schedules(model);
    this->init_visibilities(model);
    this->init_ttf(model);
    this->messages = model->_messages;
}

void Viewmodel::init_colors() {
    std::vector<unsigned> c;
    for (int i = 0; i < this->n_jobs; ++i) {
        c.push_back((360 / (this->n_jobs)) * i);
    }
    int half = c.size() / 2;
    this->colors.reserve(this->n_jobs);
    for (int i = 0; i < half; ++i) {
        this->colors.push_back(c[half + i]);
        this->colors.push_back(c[i]);
    }
    if (c.size() % 2 == 1) {
        this->colors.push_back(c.back());
    }
}

void Viewmodel::init_EDF_sorted_jobs(const Model *model) {
    /* the order of the job list can't be changed because the jobs are handled by id, which is the
     * position inside the list. That means we have to sort our own list */
    this->EDF_sorted_jobs.reserve(this->n_jobs);
    for (int i = 0; i < this->n_jobs; ++i) {
        EDF_sorted_jobs.push_back(i);
    }
    std::sort(EDF_sorted_jobs.begin(), EDF_sorted_jobs.end(), [model](int a, int b) {
            return model->_jobs.at(a)->_deadline < model->_jobs.at(b)->_deadline;
        });
}

void Viewmodel::init_submissions(const Model *model) {
    /* iterate over every job and insert submission */
    for (int job_id: this->EDF_sorted_jobs) {
        const Job *job = model->_jobs.at(job_id);

        if (this->submissions.find(job->_submission_time) == this->submissions.end()) {
            this->submissions.insert({job->_submission_time, std::vector<int>()});
        }
        this->submissions[job->_submission_time].push_back(job->_id);
    }
}

void Viewmodel::init_deadlines(const Model *model) {
    /* iterate over every job and insert deadline */
    for (int job_id: this->EDF_sorted_jobs) {
        const Job *job = model->_jobs.at(job_id);

        if (this->deadlines.find(job->_deadline) == this->deadlines.end()) {
            this->deadlines.insert({job->_deadline, std::vector<int>()});
        }
        this->deadlines[job->_deadline].push_back(job->_id);
    }
}

void Viewmodel::init_schedules(const Model *model) {
    /* create schedule rect for every schedule */
    this->schedules.reserve(model->_schedules.size());
    for (std::pair<int, const Schedule *> s: model->_schedules) {
        this->schedules.emplace_back(this, s.second);
    }
}

void Viewmodel::init_visibilities(const Model *model) {
    this->visibilities.reserve(model->_cfs_visibilities.size());
    for (const CfsVisibility *v: model->_cfs_visibilities) {
        this->visibilities.emplace_back(this, nullptr);
    }
}

void Viewmodel::init_ttf(const Model *model) {
    (void) model;
    /* TODO: get rid of magic string and magic number */
    this->font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSans.ttf", 12);
    if (!this->font) {
        std::cerr << "TTF_Error: " << TTF_GetError() << std::endl;
    }
}

RGB Viewmodel::get_color(int job, float modifier) const {
    float red = 0;
    float green = 0;
    float blue = 0;

    HSV_to_RGB((float)this->colors[job], 0.7f, 0.9f * modifier, &red, &green, &blue);
    return RGB(red * 255, green * 255, blue * 255);
}

int Viewmodel::get_player_width_px() const {
    return this->config.player.width_px;
}

void Viewmodel::HSV_to_RGB(float h, float s, float v, float *r, float *g, float *b) const {

    int i;
    float f, p, q, t;
    if (s == 0) {
        /* achromatic (grey) */
        *r = *g = *b = v;
        return;
    }

    /* sector 0 to 5 */
    h /= 60;
    i = floor(h);

    /* factorial part of h */
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));
    switch(i) {
        case 0: *r = v; *g = t; *b = p; break;
        case 1: *r = q; *g = v; *b = p; break;
        case 2: *r = p; *g = v; *b = t; break;
        case 3: *r = p; *g = q; *b = v; break;
        case 4: *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

bool Viewmodel::point_inside_rect(int x, int y, const SDL_Rect *r) const {
    return x > r->x && x < r->x + r->w
        && y > r->y && y < r->y + r->h;
}

int Viewmodel::u_to_px_w(float unit) const {
    return (unit * this->unit_w);
}

int Viewmodel::u_to_px_h(float unit) const {
    return (unit * this->unit_h);
}

float Viewmodel::px_to_u_w(int pixel) const {
    return (pixel / this->unit_w);
}

float Viewmodel::px_to_u_h(int pixel) const {
    return (pixel / this->unit_h);
}

void Viewmodel::recompute_config(const Model *model) {
    this->config.player.width_u = model->_player._max_position;
    this->config.unit.width_px =
        (this->config.window.width_px - 2 * this->config.window.margin_x_px)
        / this->config.player.width_u;
    this->config.changed = true;
}
