#pragma once

#include <view/frame.h>
#include <view/drawables.h>

class WindowFrame : public Frame {
public:
    WindowFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    void update_this(const Model *model) override;
    void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
};

class PlayerFrame : public Frame {
    int shift_x = 0;
    int shift_x_max;
    int shift_x_min = 0;
public:
    PlayerFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                int height);
    void update_this(const Model *model) override;
    void draw_drawables(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const override;
    void draw_childs(SDL_Renderer *renderer, int global_offset_x, int global_offset_y, SDL_Rect *clip_rect) const override;
    void shift(int x);
    void repair_shift();
    int get_shift_position() const;
};

class SchedulerBackgroundFrame : public Frame {
public:
    SchedulerBackgroundFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                             int width, int height);
    void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, int shift_x, int shift_y, SDL_Rect *parent_clip_rect) const override;
    void update_this(const Model *model) override;
};

class PlayerGridFrame : public Frame {
    int n_lines;
public:
    PlayerGridFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                    int height);
    void update_this(const Model *model) override;
    void rescale();
};

class DeadlineFrame : public Frame {
    unsigned max_n_submissions;
    unsigned max_n_deadlines;
public:
    DeadlineFrame(const Model *model, Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width, int height);
    void update_this(const Model *model) override;
};

class SchedulerFrame : public Frame {
    SchedulerType scheduler;
public:
    SchedulerFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                   int height, SchedulerType scheduler)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height), scheduler(scheduler) {}
    ~SchedulerFrame() override;
    void update_this(const Model *model) override;
};

class VisibilityFrame : public Frame {
public:
    VisibilityFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                    int height);
    ~VisibilityFrame() override;
    void update_this(const Model *model) override;
};

class PlayerPositionFrame : public Frame {
    Line *player_line;
public:
    PlayerPositionFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                        int height);
    void update_this(const Model *model) override;
};

class SidebarFrame : public Frame {
public:
    SidebarFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                 int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    void update_this(const Model *model) override;
};

class DependencyFrame : public Frame {
public:
    DependencyFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                    int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    void update_this(const Model *model) override;
};

class EventFrame : public Frame {
public:
    EventFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
               int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    void update_this(const Model *model) override;
};
