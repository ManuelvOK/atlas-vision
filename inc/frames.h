#include <frame.h>

class WindowFrame : public Frame {
    WindowFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};

class DeadlineFrame : public Frame {
    DeadlineFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};

class VisibilityFrame : public Frame {
    VisibilityFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};

class SchedulerFrame : public Frame {
    SchedulerFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};

class SidebarFrame : public Frame {
    SidebarFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};

class DependencyFrame : public Frame {
    DependencyFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};

class EventFrame : public Frame {
    EventFrame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : Frame(viewmodel, offset_x, offset_y) {}
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void update_this(const Model *model);
};
