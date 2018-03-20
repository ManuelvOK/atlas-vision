#pragma once

#include <SDL2/SDL.h>

#include <controller.h>
#include <frames.h>
#include <viewmodel.h>

class View {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    WindowFrame *window_frame = nullptr;
    const Model *model = nullptr;

    Viewmodel *viewmodel = nullptr;

    void create_frame_hierarchy();
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
    float position_in_player(int x, int y);

    /**
     * The Window size has changed, that means the rendering has to adjust scaling.
     *
     * @param width
     *   window width
     * @param height
     *   window height
     */
    void update_window(int width, int height);
};
