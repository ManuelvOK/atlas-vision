#pragma once

#include <frame.h>
#include <drawables.h>

class WindowFrame : public Frame {
public:
    WindowFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    virtual void update_this(const Model *model);
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
};

class PlayerFrame : public Frame {
public:
    PlayerFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    virtual void update_this(const Model *model);
};

class SchedulerBackgroundFrame : public Frame {
public:
    SchedulerBackgroundFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                             int width, int height);
    virtual void update_this(const Model *model);
};

class PlayerGridFrame : public Frame {
public:
    PlayerGridFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                    int height);
    virtual void update_this(const Model *model);
};

class DeadlineFrame : public Frame {
    unsigned max_n_submissions;
    unsigned max_n_deadlines;
public:
    DeadlineFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                  int height);
    virtual void update_this(const Model *model);
};

class SchedulerFrame : public Frame {
    SchedulerType scheduler;
public:
    SchedulerFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                   int height, SchedulerType scheduler)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height), scheduler(scheduler) {}
    virtual ~SchedulerFrame();
    virtual void update_this(const Model *model);
};

class VisibilityFrame : public Frame {
public:
    VisibilityFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                    int height);
    virtual ~VisibilityFrame();
    virtual void update_this(const Model *model);
};

class PlayerPositionFrame : public Frame {
    Line *player_line;
public:
    PlayerPositionFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                        int height);
    virtual void update_this(const Model *model);
};

class SidebarFrame : public Frame {
public:
    SidebarFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                 int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    virtual void update_this(const Model *model);
};

class DependencyFrame : public Frame {
public:
    DependencyFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                    int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    virtual void update_this(const Model *model);
};

class EventFrame : public Frame {
public:
    EventFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
               int height)
        : Frame(parent, viewmodel, offset_x, offset_y, width, height) {}
    virtual void update_this(const Model *model);
};
