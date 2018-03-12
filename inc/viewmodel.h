#pragma once

#include <vector>
#include <map>

#include <SDL2/SDL.h>

#include <schedule_rect.h>
#include <vision_config.h>

class Viewmodel {
    bool colors_initialised = false;

    ViewConfig config;

    /**
     * convert HSV color values to RGB ones
     *
     * @param h
     *   h value for HSV color code
     * @param s
     *   s value for HSV color code
     * @param v
     *   v value for HSV color code
     * @param[out] r
     *   r value for RGB color code
     * @param[out] g
     *   g value for RGB color code
     * @param[out] b
     *   b value for RGB color code
     */
    void HSV_to_RGB(float h, float s, float v, float *r, float *g, float *b) const;

  public:
    SDL_Window *window;
    SDL_Renderer *renderer;

    int n_jobs;
    int n_schedules;
    std::vector<Schedule_rect> schedules;
    std::vector<Schedule_rect> EDF_schedules;
    std::vector<struct job_rect> deadlines_render_positions;
    std::vector<struct job_rect> deadline_history_render_positions;
    std::vector<struct job_rect> submission_render_positions;
    std::map<int, std::vector<int>> deadlines;
    std::map<int, std::vector<int>> submissions;
    std::vector<unsigned> colors;

    Viewmodel() : config() {}

    /**
     * initialise color presets
     *
     * @param n_jobs
     *   number of jobs to calculate color preset for
     */
    void init_colors();

    /**
     * set render color to preset colors have to be initialised first
     *
     * @param job
     *   job number
     * @param modifier
     *   color alpha
     */
    void set_color(int job, float modifier = 1.0);

    /**
     * get rgb values of the color for a given job
     *
     * @param job
     *   number of job to get color for
     * @param modifier
     *   color alpha
     * @param[out] r
     *   red value to fill
     * @param[out] g
     *   g value to fill
     * @param[out] b
     *   b value to fill
     */
    void get_color(int job, float modifier, int *r, int *g, int *b) const;

    /**
     * check if point is inside a SDL_Rect
     *
     * @param x
     *   x coordinate of point to check
     * @param y
     *   y coordinate of point to check
     *
     * @return
     *   true if Point is inside SDL_Rect. Otherwise False
     */
    bool point_inside_rect(int x, int y, const SDL_Rect *r);

    int u_to_px_w(float unit) const;
    int u_to_px_h(float unit) const;
    float px_to_u_w(int pixel) const;
    float px_to_u_h(int pixel) const;

    void recompute_config(const Model *model);
};
