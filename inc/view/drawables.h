#pragma once

#include <array>

#include <view/drawable.h>
#include <model/job.h>
#include <model/message.h>
#include <model/rgb.h>
#include <model/schedule.h>

class Viewmodel;
class Arrow : public Drawable {

    const Job *job;
    const std::array<short,9> arrow_coords_x;
    const std::array<short,9> arrow_coords_y;

public:
    RGB color;
    int x;
    int y;

    Arrow(const Viewmodel *viewmodel, const Job *job, std::array<short,9> arrow_coords_x, std::array<short,9> arrow_coords_y, int x = 0, int y = 0);

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;
    bool is_visible(int timestamp) const override;
};

class DeadlineArrow : public Arrow {
public:
    DeadlineArrow(const Viewmodel *viewmodel, const Job *job, int x, int y)
        : Arrow(viewmodel, job,
                {-10, 10, 10, 40, 50, 0, -50, -40, -10},
                {0, 0, 80, 50, 60, 110, 60, 50, 80},
                x, y) {};
};

class SubmissionArrow : public Arrow {
public:
    SubmissionArrow(const Viewmodel *viewmodel, const Job *job, int x, int y) :
        Arrow(viewmodel, job,
              {-10, 10, 10, 40, 50, 0, -50, -40, -10},
              {0, 0, -80, -50, -60, -110, -60, -50, -80},
              x, y) {};
};

class Rect : public Drawable {
public:
    SDL_Rect rect = {0,0,0,0};
    RGB color;
    RGB border_color;
    bool border = false;

    Rect(const Viewmodel *viewmodel) : Drawable(viewmodel) {}

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

class ScheduleRect : public Rect {
    const Schedule *schedule;
public:
    SchedulerType scheduler = SchedulerType::CFS;
    int begin = 0;
    int time = 0;
    bool visible = false;

    ScheduleRect(const Viewmodel *viewmodel, const Schedule *schedule);

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;

    void recalculate_position();
};

class SchedulerRect : public Rect {
public:
    SchedulerRect(const Viewmodel *viewmodel) : Rect(viewmodel) {}

};

class JobRect : public Rect {
    const Job *job;
public:
    JobRect(const Viewmodel *viewmodel, const Job *job, int offset_x = 0, int offset_y = 0);
    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;
    bool is_visible(int timestamp) const override;
};

class Line : public Drawable {
public:
    int begin_x = 0;
    int begin_y = 0;
    int end_x = 0;
    int end_y = 0;

    RGB color;
    Line(const Viewmodel *viewmodel) : Drawable(viewmodel) {}
    Line(const Viewmodel *viewmodel, int begin_x, int begin_y, int end_x, int end_y)
        : Drawable(viewmodel), begin_x(begin_x), begin_y(begin_y), end_x(end_x), end_y(end_y) {}
    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

class JobDependencyLine : public Line {
public:
    JobRect *r1;
    JobRect *r2;
    JobDependencyLine(const Viewmodel *viewmodel, JobRect *r1, JobRect *r2);
};

class VisibilityLine : public Line {
    const Schedule *schedule;
public:
    bool visible = false;
    const Viewmodel *viewmodel;

    VisibilityLine(Viewmodel *viewmodel, const Schedule *schedule) : Line(viewmodel), schedule(schedule) {}
    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

class Text : public Drawable {
protected:
    SDL_Surface *surface = nullptr;
    int offset_x;
    int offset_y;
public:
    Text(Viewmodel *viewmodel, int offset_x, int offset_y);
    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    int width() const;
    int height() const;
};

class SimpleText : public Text {
    const std::string text;
public:
    SimpleText(Viewmodel *viewmodel, const std::string text, int offset_x, int offset_y);
    ~SimpleText();
    bool is_visible(int timestamp) const override;

    void set_offset_x(int offset_x);
    void set_offset_y(int offset_y);
};

class MessageText : public Text {
    const Message *message;
    SDL_Surface *surface_active;
    SDL_Surface *surface_inactive;
public:
    MessageText(Viewmodel *viewmodel, const Message *message, int width, int offset_y);
    ~MessageText();
    void update(const Model *model) override;

    bool is_visible(int timestamp) const override;
};
