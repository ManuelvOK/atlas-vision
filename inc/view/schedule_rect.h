#pragma once

#include <utility>

#include <SDL2/SDL.h>

#include <model/schedule.h>
#include <view/drawables.h>
#include <view/view_config.h>

class DependencyClass {
protected:
    bool changed = true;
public:
    void dependency_changed() {this->changed = true;}
};

template <typename T>
class DependencyValue {
    DependencyClass *dependency_class;
    T value;
public:
    DependencyValue(DependencyClass *c, T value) : dependency_class(c), value(value) {}
    operator T() const {return this->value;}
    DependencyValue &operator=(T value) {
        if (value == this->value) {
            return *this;
        }
        dependency_class->dependency_changed();
        this->value = value;
        return *this;
    }
};

class Schedule_rect : public DependencyClass {
    SDL_Rect m_render_position = {0, 0, 0, 0};
    const ViewConfig *config;

public:
    int job_id;
    DependencyValue<float> x;
    DependencyValue<float> y;
    DependencyValue<float> w;
    DependencyValue<float> h;
    DependencyValue<SchedulerType> scheduler;
    bool visible = true;

    Schedule_rect(const ViewConfig *config, int job_id, SchedulerType scheduler, float x = 0.0,
                  float y = 0.0, float w = 0.0, float h = 0.0)
        : config(config), job_id(job_id), x(this, x), y(this, y), w(this, w), h(this, h),
          scheduler(this, scheduler) {}

    Schedule_rect(Schedule_rect &&o) noexcept
        : config(std::move(o.config)), job_id(std::move(o.job_id)), x(this, std::move(o.x)),
          y(this, std::move(o.y)), w(this, std::move(o.w)), h(this, std::move(o.h)),
          scheduler(this, std::move(o.scheduler)) {}

    ScheduleRect *drawable();
    void calculate_render_position();

};
