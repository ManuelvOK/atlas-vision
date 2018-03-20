#include <viewmodel.h>

#include <iostream>
#include <algorithm>

Viewmodel::Viewmodel(const Model *model) :
    config(), EDF_sorted_jobs(), schedules(), EDF_schedules(), deadlines(), submissions(), colors() {
    this->n_jobs = model->jobs.size();
    this->n_schedules = model->schedules.size();
    this->init_colors();
    this->init_EDF_sorted_jobs(model);
    this->init_submissions(model);
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
            return model->jobs.at(a).deadline < model->jobs.at(b).deadline;
        });
}

void Viewmodel::init_submissions(const Model *model) {
    /* iterate over every job and insert submission */
    for (int job_id: this->EDF_sorted_jobs) {
        const Job &job = model->jobs.at(job_id);

        if (this->submissions.find(job.submission_time) == this->submissions.end()) {
            this->submissions.insert({job.submission_time, std::vector<int>()});
        }
        this->submissions[job.submission_time].push_back(job.id);
    }
}

RGB Viewmodel::get_color(int job, float modifier) const {
    float red = 0;
    float green = 0;
    float blue = 0;

    HSV_to_RGB((float)this->colors[job], 0.7f, 0.9f * modifier, &red, &green, &blue);
    return RGB(red * 255, green * 255, blue * 255);
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
    return (unit * this->config.unit.width_px);
}

int Viewmodel::u_to_px_h(float unit) const {
    return (unit * this->config.unit.height_px);
}

float Viewmodel::px_to_u_w(int pixel) const {
    return (pixel / this->config.unit.width_px);
}

float Viewmodel::px_to_u_h(int pixel) const {
    return (pixel / this->config.unit.height_px);
}

void Viewmodel::recompute_config(const Model *model) {
    this->config.player.width_u = model->player.max_position;
    this->config.unit.width_px =
        (this->config.window.width_px - 2 * this->config.window.margin_x_px)
        / this->config.player.width_u;
    this->config.changed = true;
}
