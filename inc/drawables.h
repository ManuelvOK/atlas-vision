#include <drawable.h>
#include <rgb.h>

#include <array>

class Arrow : public Drawable {
    RGB color;

    std::array<short,9> x;
    std::array<short,9> y;

public:
    SDL_Rect position = {0,0,0,0};

    Arrow(std::array<short,9>, std::array<short,9>) : x(x), y(y) {}

    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const;
};

class DeadlineArrow : public Arrow {
    DeadlineArrow() : Arrow({-10, 10, 10, 40, 50, 0, -50, -40, -10},
                            {0, 0, 80, 50, 60, 110, 60, 50, 80}) {};
};

class SubmissionArrow : public Arrow {
    SubmissionArrow() : Arrow({-10, 10, 10, 40, 50, 0, -50, -40, -10},
                              {0, 0, -80, -50, -60, -110, -60, -50, -80}) {};
};

class Rect : public Drawable {
    SDL_Rect rect = {0,0,0,0};
    RGB color;
    RGB border_color;
    bool border = false;

public:
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const;
};

class ScheduleRect : public Rect {

};

class SchedulerRect : public Rect {

};

class Line : public Drawable {
    int begin_x = 0;
    int begin_y = 0;
    int end_x = 0;
    int end_y = 0;

    RGB color;
public:
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const;
};
