#include <frame.h>

class WindowFrame : public Frame {
    WindowFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};

class SchedulerBackgroundFrame : public Frame {
    SchedulerBackgroundFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}

};

class PlayerGridFrame : public Frame {
    PlayerGridFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}

};

class DeadlineFrame : public Frame {
    /* TODO: documentation */
    void update_submission_positions();
public:
    DeadlineFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};

class SchedulerFrame : public Frame {
    void SchedulerFrame::update_schedule_render_position(const Model *model, const Schedule &schedule);
public:
    SchedulerFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};

class VisibilityFrame : public Frame {
    VisibilityFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};

class PlayerPositionFrame : public Frame {
    PlayerPositionFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
};

class SidebarFrame : public Frame {
    SidebarFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};

class DependencyFrame : public Frame {
    DependencyFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};

class EventFrame : public Frame {
    EventFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(parent, viewmodel, offset_x, offset_y) {}
    virtual void update_this(const Model *model);
};
