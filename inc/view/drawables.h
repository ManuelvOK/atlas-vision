#pragma once

#include <array>

#include <view/drawable.h>
#include <model/job.h>
#include <model/message.h>
#include <model/rgb.h>
#include <model/schedule.h>

class Viewmodel;
/**
 * A big Arrow related to a job
 */
class Arrow : public Drawable {
    const Job *_job; /**< related job */
    const std::array<short,9> _arrow_coords_x; /* x coordinates of arrow shape */
    const std::array<short,9> _arrow_coords_y; /* y coordinates of arrow shape */

public:
    RGB _color; /**< color of Arrow */
    int _x; /**< horizontal position */
    int _y; /**< vertical position */

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param job
     *   related job
     * @param arrow_coords_x
     *   x coordinates of arrow shape
     * @param arrow_coords_y
     *   y coordinates of arrow shape
     * @param x
     *   horizontal position inside frame
     * @param y
     *   vertical position inside frame
     */
    Arrow(const Viewmodel *viewmodel, const Job *job, std::array<short,9> arrow_coords_x, std::array<short,9> arrow_coords_y, int x = 0, int y = 0);

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

/**
 * Arrow that points downwards and therefore is used for deadline visualisation
 */
class DeadlineArrow : public Arrow {
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param job
     *   related job
     * @param x
     *   horizontal position inside frame
     * @param y
     * vertical position inside frame
     */
    DeadlineArrow(const Viewmodel *viewmodel, const Job *job, int x, int y)
        : Arrow(viewmodel, job,
                {-10, 10, 10, 40, 50, 0, -50, -40, -10},
                {0, 0, 80, 50, 60, 110, 60, 50, 80},
                x, y) {};
};

/**
 * Arrow that points upwards and therefore is used for submission visualisation
 */
class SubmissionArrow : public Arrow {
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param job
     *   related job
     * @param x
     *   horizontal position inside frame
     * @param y
     * vertical position inside frame
     */
    SubmissionArrow(const Viewmodel *viewmodel, const Job *job, int x, int y) :
        Arrow(viewmodel, job,
              {-10, 10, 10, 40, 50, 0, -50, -40, -10},
              {0, 0, -80, -50, -60, -110, -60, -50, -80},
              x, y) {};
};

/**
 * A simple rectangle
 */
class Rect : public Drawable {
public:
    SDL_Rect _rect = {0,0,0,0}; /**< position inside frame and dimensions */
    RGB _color; /**< background color */
    RGB _border_color; /**< border color */
    bool _border = false; /**< border flag */

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     */
    Rect(const Viewmodel *viewmodel) : Drawable(viewmodel) {}

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

/**
 * A simple rectangle related to a specific schedule
 */
class ScheduleRect : public Rect {
    const Schedule *_schedule; /**< related schedule */
public:
    SchedulerType _scheduler = SchedulerType::CFS; /**< Scheduler */
    int _begin = 0; /**< begin timestamp of schedule */
    int _time = 0; /**< end timestamp of schedule */
    bool _visible = false; /**< visibility flag */

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @schedule
     *   related schedule
     */
    ScheduleRect(const Viewmodel *viewmodel, const Schedule *schedule);

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;

    /**
     * recalculate the rectangles position based on the schedule data at the current timestamp
     */
    void recalculate_position();
};

/**
 * Background rectangle for a scheduler
 */
class SchedulerRect : public Rect {
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     */
    SchedulerRect(const Viewmodel *viewmodel) : Rect(viewmodel) {}
};

/**
 * A simple rect related to a job
 */
class JobRect : public Rect {
    const Job *_job; /**< related job */
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param job
     *   related job
     * @param offset_x
     *   horizontal offset in frame
     * @param offset_y
     *   vertical offset in frame
     */
    JobRect(const Viewmodel *viewmodel, const Job *job, int offset_x = 0, int offset_y = 0);

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

/**
 * A Simple line
 */
class Line : public Drawable {
public:
    int _begin_x = 0; /**< x coordinate of beginning of the line */
    int _begin_y = 0; /**< y coordinate of beginning of the line */
    int _end_x = 0; /**< x coordinate of end of the line */
    int _end_y = 0; /**< y coordinate of end of the line */
    RGB _color; /**< line color */

    /**
     * Constructor
     * This Constructor initialises a zero-length line at (0,0)
     *
     * @param viewmodel
     *   applications viewmodel
     */
    Line(const Viewmodel *viewmodel) : Drawable(viewmodel) {}

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param begin_x
     *   x-coordinate of beginning of the line
     * @param begin_y
     *   y-coordinate of beginning of the line
     * @param end_x
     *   x-coordinate of end of the line
     * @param end_y
     *   y-coordinate of end of the line
     */
    Line(const Viewmodel *viewmodel, int begin_x, int begin_y, int end_x, int end_y)
        : Drawable(viewmodel), _begin_x(begin_x), _begin_y(begin_y), _end_x(end_x), _end_y(end_y) {}

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

/**
 * line that is drawn between two JobRects visualising a dependency
 */
class JobDependencyLine : public Line {
public:
    JobRect *_r1; /**< JobRect of first Job */
    JobRect *_r2; /**< JobRect of second Job */

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param r1
     *   JobRect of first Job
     * @param r2
     *   JobRect of second Job
     */
    JobDependencyLine(const Viewmodel *viewmodel, JobRect *r1, JobRect *r2);
};

/**
 * line that visualises the visibility of a schedule for the CFS scheduler.
 *
 * It is drawn from the current player position at the CFS scheduler to the beginning of the related
 * schedule
 */
class VisibilityLine : public Line {
    const Schedule *_schedule; /**< related schedule */
public:
    bool _visible = false; /**< visibility flag */

    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param schedule
     *   related schedule
     */
    VisibilityLine(Viewmodel *viewmodel, const Schedule *schedule) : Line(viewmodel), _schedule(schedule) {}

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    bool is_visible(int timestamp) const override;
};

/**
 * A Text view
 */
class Text : public Drawable {
protected:
    SDL_Surface *_surface = nullptr; /**< SDL surface to put text on */
    int _offset_x; /**< horizontal offset in frame */
    int _offset_y; /**< vertical offset in frame */
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param offset_x
     *   horizontal offset in frame
     * @param offset_y
     *   vertical offset in frame
     */
    Text(Viewmodel *viewmodel, int offset_x, int offset_y);

    void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const override;

    /**
     * get width of text field
     *
     * @returns
     *   width of textfield in pixel
     */
    int width() const;

    /**
     * get height of text field
     *
     * @returns
     *   height of textfield in pixel
     */
    int height() const;
};

/**
 * a simple blended text view
 */
class SimpleText : public Text {
    const std::string _text; /**< text to display */
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param text
     *   text to display
     * @param offset_x
     *   horizontal offset in frame
     * @param offset_y
     *   vertical offset in frame
     */
    SimpleText(Viewmodel *viewmodel, const std::string text, int offset_x, int offset_y);

    /**
     * Destructor
     */
    ~SimpleText();

    bool is_visible(int timestamp) const override;

    /**
     * setter for _offset_x
     *
     * @param offset_x
     *   horizontal offset in frame
     */
    void set_offset_x(int offset_x);

    /**
     * setter for _offset_y
     *
     * @param offset_y
     *   vertical offset in frame
     */
    void set_offset_y(int offset_y);
};

/**
 * View for simulation message text
 */
class MessageText : public Text {
    const Message *_message; /**< related simulation message */
    SDL_Surface *_surface_active; /**< prerendered text for being active */
    SDL_Surface *_surface_inactive; /**< prerendered text for being inactive */
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param message
     *   relates simulation message
     * @param width
     *   maximal width of textfield. Important for wrapping
     * @param offset_y
     *   vertical offset in frame
     */
    MessageText(Viewmodel *viewmodel, const Message *message, int width, int offset_y);

    /**
     * Destructor
     */
    ~MessageText();

    void update(const Model *model) override;

    bool is_visible(int timestamp) const override;
};
