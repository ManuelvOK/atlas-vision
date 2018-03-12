#include <viewmodel.h>

void Viewmodel::init_colors() {
    std::vector<unsigned> c;
    for (int i = 0; i < n_jobs; ++i) {
        c.push_back((360 / (n_jobs)) * i);
    }
    int half = c.size() / 2;
    this->colors.reserve(n_jobs);
    for (int i = 0; i < half; ++i) {
        this->colors.push_back(c[half + i]);
        this->colors.push_back(c[i]);
    }
    if (c.size() % 2 == 1) {
        this->colors.push_back(c.back());
    }
    this->colors_initialised = true;
}

void Viewmodel::set_color(int job, float modifier) {
    if (not this->colors_initialised) {
        return;
    }
    float r = 0;
    float g = 0;
    float b = 0;

    this->HSV_to_RGB((float)this->colors[job], 0.7f, 0.9f * modifier, &r, &g, &b);
    SDL_SetRenderDrawColor(this->renderer, r * 255, g * 255, b * 255, 255);
}

void Viewmodel::get_color(int job, float modifier, int *r, int *g, int *b) const {
    float red = 0;
    float green = 0;
    float blue = 0;

    HSV_to_RGB((float)this->colors[job], 0.7f, 0.9f * modifier, &red, &green, &blue);
    *r = red * 255;
    *g = green * 255;
    *b = blue * 255;
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
