#include <models/interface_model.h>

#include <cmath>

#include <SDL_GUI/gui/util.h>

#include <models/schedule.h>

void InterfaceModel::init_colors(int n_jobs) {
    this->_colors.clear();
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

SDL_GUI::RGB InterfaceModel::get_color(int job, float modifier) const {
    float red = 0;
    float green = 0;
    float blue = 0;

    SDL_GUI::util::HSV_to_RGB((float)this->_colors[job], 0.7f, 0.9f * modifier,
                              &red, &green, &blue);
    return SDL_GUI::RGB(red * 255, green * 255, blue * 255);
}

int InterfaceModel::px_width(float unit) const {
    return std::ceil(unit * this->_player_model->zoom());
}

int InterfaceModel::px_height(float unit) const {
    return std::ceil(unit * this->_unit_height);
}

float InterfaceModel::unit_width(int pixel) const {
    return std::ceil(pixel / this->_player_model->zoom());
}

float InterfaceModel::unit_height(int pixel) const {
    return std::ceil(pixel / this->_unit_height);
}

int InterfaceModel::scheduler_offset(SchedulerType t) const {
    static const std::map<SchedulerType, int> position = {{SchedulerType::ATLAS, 0}, {SchedulerType::recovery, 1}, {SchedulerType::CFS, 2}};
    int scheduler_height_before = position.at(t) * interface_config.unit.height_px;
    int distance_before = (1 + position.at(t)) * interface_config.player.scheduler_distance_px;
    return scheduler_height_before + distance_before;
}

int InterfaceModel::core_rect_height() const {
    static const int n_schedulers = 3;
    int scheduler_height = n_schedulers * interface_config.unit.height_px;
    int distance = (1 + n_schedulers) * interface_config.player.scheduler_distance_px;
    return scheduler_height + distance;
}
