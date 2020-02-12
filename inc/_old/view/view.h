#pragma once

#include <SDL2/SDL.h>

#include <controller.h>
#include <util/attribute_tree.h>
#include <view/frames.h>
#include <view/viewmodel.h>

//Tree<Frame> foo;

class View {
    SDL_Window *_window = nullptr; /**< SDL Window to display view */
    SDL_Renderer *_renderer = nullptr; /**< SDL Renderer to draw on */

    WindowFrame *_window_frame = nullptr; /**< Applications root frame */
    PlayerFrame *_player_frame = nullptr; /**<  */
    SidebarFrame *_sidebar_frame = nullptr; /**<  */
    SchedulerBackgroundFrame *_scheduler_background_frame = nullptr; /**<  */
    PlayerGridFrame *_player_grid_frame = nullptr; /**<  */
    DeadlineFrame *_deadline_frame = nullptr; /**<  */
    SchedulerFrame *_ATLAS_frame = nullptr; /**<  */
    SchedulerFrame *_recovery_frame = nullptr; /**<  */
    SchedulerFrame *_CFS_frame = nullptr; /**<  */
    VisibilityFrame *_visibility_frame = nullptr; /**<  */
    PlayerPositionFrame *_player_position_frame = nullptr; /**<  */
    LegendFrame *_legend_frame = nullptr; /**<  */
    DependencyFrame *_dependency_frame = nullptr; /**<  */
    EventFrame *_event_frame = nullptr; /**<  */
    const Model *_model = nullptr; /**<  */

    Viewmodel *_viewmodel = nullptr; /**<  */

    /**
     * initialise the hierarchy between the frames
     */
    void create_frame_hierarchy();

    /**
     * synchronise schedule rects with schedules at current timestamp
     */
    void update_schedules();

    /**
     * synchronise CFS visibilities at current timestamp
     */
    void update_visibilities();

    Tree<Frame> _frame_hierarchy;
public:

    /**
     * initialise graphics
     */
    View(const Model *model);

    ~View();

    /**
     * render a frame from a given model.
     *
     * @param model
     *   application model to render
     */
    void render();

    /**
     * calculate positions of boxes for vision.
     * TODO: Is this needed any longer?
     *
     * This is for performance reasons. Since the jobs don't change during the execution of the
     * application positioning can be precomputed.
     *
     * @param model
     *   application model to precompute vision for
     */
    void calculate();

    /**
     * interprete mouse position and get hovered job
     *
     * @param x
     *   x coordinate of mouse position
     * @param y
     *   y coordinate of mouse position
     * @param model
     *   application model
     *
     * @return
     *   id of hovered job or -1 if no job is hovered
     */
    int get_hovered_job(int x, int y);

    /**
     * get player position under mouse cursor
     *
     * @param x
     *   x coordinate of mouse position
     * @param y
     *   y coordinate of mouse position
     * @param model
     *   application model
     *
     * @return
     *   player position unter mouse pointer
     */
    int position_in_player(int x, int y) const;

    /**
     * The Window size has changed, that means the rendering has to adjust scaling.
     *
     * @param width
     *   window width
     * @param height
     *   window height
     */
    void update_window(int width, int height);

    void rescale(float factor);

    void shift_player(int offset);
};
