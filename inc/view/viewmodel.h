#pragma once

#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <model/model.h>
#include <model/rgb.h>
#include <view/schedule_rect.h>
#include <view/view_config.h>

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

    /**
     * TODO: documentation
     */
    void init_deadlines(const Model *model);

    /**
     * TODO: documentation
     */
    void init_schedules(const Model *model);

    /**
     * TODO: documentation
     */
    void init_visibilities(const Model *model);

    /**
     * init TTF related stuff
     */
    void init_ttf(const Model *model);

  public:
    ViewConfig config;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font *font = nullptr;

    float unit_w;
    float unit_w_min;
    float unit_h;

    bool rescaled = false;

    int n_jobs = 0;
    int n_schedules = 0;
    std::vector<int> EDF_sorted_jobs;
    std::vector<ScheduleRect> schedules;
    std::map<int, std::vector<int>> deadlines;
    std::map<int, std::vector<int>> submissions;
    std::vector<Message> messages;
    std::vector<VisibilityLine> visibilities;
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
    int get_player_width_px() const;
};
