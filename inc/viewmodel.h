#pragma once

#include <vector>
#include <map>

#include <SDL2/SDL.h>

#include <schedule_rect.h>
#include <view_config.h>
#include <model.h>
#include <rgb.h>

class Viewmodel {
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

    /**
     * initialise color presets
     *
     * @param n_jobs
     *   number of jobs to calculate color preset for
     */
    void init_colors();

    /**
     * TODO: documentation
     */
    void init_EDF_sorted_jobs(const Model *model);

    /**
     * TODO: documentation
     */
    void init_submissions(const Model *model);

  public:
    ViewConfig config;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    int n_jobs = 0;
    int n_schedules = 0;
    std::vector<int> EDF_sorted_jobs;
    std::vector<Schedule_rect> schedules;
    std::vector<Schedule_rect> EDF_schedules;
    std::map<int, std::vector<int>> deadlines;
    std::map<int, std::vector<int>> submissions;
    std::vector<unsigned> colors;

    Viewmodel(const Model *model);


    /**
     * get rgb values of the color for a given job
     *
     * @param job
     *   number of job to get color for
     * @param modifier
     *   color alpha
     *
     * @return RGB color object of job color
     */
    RGB get_color(int job, float modifier = 1.0) const;

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
    bool point_inside_rect(int x, int y, const SDL_Rect *r) const;

    int u_to_px_w(float unit) const;
    int u_to_px_h(float unit) const;
    float px_to_u_w(int pixel) const;
    float px_to_u_h(int pixel) const;

    void recompute_config(const Model *model);
};
