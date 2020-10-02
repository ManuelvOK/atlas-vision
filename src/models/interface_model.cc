#include <models/interface_model.h>

#include <cmath>

#include <SDL_GUI/inc/gui/util.h>


void InterfaceModel::init_colors(int n_jobs) {
    std::vector<unsigned> c;
    for (int i = 0; i < n_jobs; ++i) {
        c.push_back((360 / (n_jobs)) * i);
    }
    int half = c.size() / 2;
    this->_colors.reserve(n_jobs);
    for (int i = 0; i < half; ++i) {
        this->_colors.push_back(c[half + i]);
        this->_colors.push_back(c[i]);
    }
    if (c.size() % 2 == 1) {
        this->_colors.push_back(c.back());
    }
}

void InterfaceModel::set_unit_width(float unit_width) {
    this->_unit_width = std::max(unit_width, this->_unit_width_min);
}

void InterfaceModel::set_unit_width_min(float unit_width_min) {
    this->_unit_width_min = unit_width_min;
    this->set_unit_width(this->_unit_width);
}

SDL_GUI::RGB InterfaceModel::get_color(int job, float modifier) const {
    float red = 0;
    float green = 0;
    float blue = 0;

    SDL_GUI::util::HSV_to_RGB((float)this->_colors[job], 0.7f, 0.9f * modifier, &red, &green, &blue);
    return SDL_GUI::RGB(red * 255, green * 255, blue * 255);
}

int InterfaceModel::px_width(float unit) const {
    return std::ceil(unit * this->_unit_width);
}

int InterfaceModel::px_height(float unit) const {
    return std::ceil(unit * this->_unit_height);
}

float InterfaceModel::unit_width(int pixel) const {
    return std::ceil(pixel / this->_unit_width);
}

float InterfaceModel::unit_height(int pixel) const {
    return std::ceil(pixel / this->_unit_height);
}
