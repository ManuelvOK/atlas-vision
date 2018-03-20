#pragma once
#include <drawable.h>
#include <rgb.h>

#include <array>

class Arrow : public Drawable {

    const std::array<short,9> arrow_coords_x;
    const std::array<short,9> arrow_coords_y;

public:
    RGB color;
    int x;
    int y;

    Arrow(std::array<short,9> arrow_coords_x, std::array<short,9> arrow_coords_y, int x = 0,
          int y = 0);

    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const;
};

class DeadlineArrow : public Arrow {
    DeadlineArrow(int x, int y) : Arrow({-10, 10, 10, 40, 50, 0, -50, -40, -10},
                                        {0, 0, 80, 50, 60, 110, 60, 50, 80},
                                        x, y) {};
};

class SubmissionArrow : public Arrow {
public:
    SubmissionArrow(int x, int y) : Arrow({-10, 10, 10, 40, 50, 0, -50, -40, -10},
                                          {0, 0, -80, -50, -60, -110, -60, -50, -80},
                                          x, y) {};
};

class Rect : public Drawable {
public:
    SDL_Rect rect = {0,0,0,0};
    RGB color;
    RGB border_color;
    bool border = false;

    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const;
};

class ScheduleRect : public Rect {

};

class SchedulerRect : public Rect {

};

class Line : public Drawable {
public:
    int begin_x = 0;
    int begin_y = 0;
    int end_x = 0;
    int end_y = 0;

    RGB color;
    Line() = default;
    Line(int begin_x, int begin_y, int end_x, int end_y)
        : begin_x(begin_x), begin_y(begin_y), end_x(end_x), end_y(end_y) {}
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const;
};
