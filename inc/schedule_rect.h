#pragma once

#include <utility>

#include <SDL2/SDL.h>

#include <vision_config.h>

class Dependency_class {
protected:
    bool changed = true;
public:
    void dependency_changed() {this->changed = true;}
};

template <typename T>
class Dependency_value {
    Dependency_class *dependency_class;
    T value;
public:
    Dependency_value(Dependency_class *c, T value) : dependency_class(c), value(value) {}
    operator T() const {return this->value;}
    Dependency_value &operator=(T value) {
        if (value == this->value) {
            return *this;
        }
        dependency_class->dependency_changed();
        this->value = value;
        return *this;
    }
};

class Schedule_rect : public Dependency_class {
    SDL_Rect m_render_position = {0, 0, 0, 0};
    const Vision_config *config;

public:
    int job_id;
    Dependency_value<float> x;
    Dependency_value<float> y;
    Dependency_value<float> w;
    Dependency_value<float> h;
    bool visible = true;

    Schedule_rect(const Vision_config *config, int job_id, float x = 0.0, float y = 0.0,
                  float w = 0.0, float h = 0.0)
        : config(config), job_id(job_id), x(this, x), y(this, y), w(this, w), h(this, h) {}

    Schedule_rect(Schedule_rect &&o) noexcept
        : config(std::move(o.config)), job_id(std::move(o.job_id)), x(this, std::move(o.x)),
          y(this, std::move(o.y)), w(this, std::move(o.w)), h(this, std::move(o.h)) {}

    SDL_Rect *render_position();
    void calculate_render_position();

};