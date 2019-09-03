#pragma once

#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <model/model.h>
#include <model/rgb.h>
#include <view/schedule_rect.h>
#include <view/view_config.h>

/**
 * applications Viewmodel
 * TODO: this should not exist and has to be reworked
 */
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
     * Sort Jobs EDF-wise
     *
     * @param model
     *   applications model
     */
    void init_EDF_sorted_jobs(const Model *model);

    /**
     * create submissions for every simulated job
     *
     * @param model
     *   applications model
     */
    void init_submissions(const Model *model);

    /**
     * create deadline for every simulated job
     *
     * @param model
     *   applications model
     */
    void init_deadlines(const Model *model);

    /**
     * create schedule rects for every simulated schedule
     *
     * @param model
     *   applications model
     */
    void init_schedules(const Model *model);

    /**
     * create visibility structures for every simulated CFS visibility
     *
     * @param model
     *   applications model
     */
    void init_visibilities(const Model *model);

    /**
     * init TTF related stuff
     *
     * @param model
     *   applications model
     */
    void init_ttf(const Model *model);

  public:
    ViewConfig config; /**< config of view */
    SDL_Window *window = nullptr; /**< SDL_Window to display view */
    SDL_Renderer *renderer = nullptr; /**< SDL_Renderer to draw on */
    TTF_Font *font = nullptr; /**< Font to use */

    float unit_w; /**< current unit to pixel ratio of width*/
    float unit_w_min; /**< minimal unit to pixel ratio of width */
    float unit_h; /**< current unit to pixel ratio of height */

    bool rescaled = false; /**< flag determining rescaling */

    int n_jobs = 0; /**< number of jobs */
    int n_schedules = 0; /**< number of schedules */
    const std::vector<Job *> *jobs; /**< simulation jobs */
    std::vector<int> EDF_sorted_jobs; /**< jobs sorted EDF-wise */
    std::vector<ScheduleRect> schedules; /**< Schedule view rectangles */
    std::map<int, std::vector<int>> deadlines; /**< simulation deadlines */
    std::map<int, std::vector<int>> submissions; /**< simulation submissions */
    std::vector<Message *> messages; /**< simulation messages */
    std::vector<VisibilityLine> visibilities; /**< job visibilities for CFS */
    std::vector<unsigned> colors; /**< job colors */

    /**
     * Constructor
     *
     * @param model
     *   applications model
     */
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

    /**
     * convert width unit to pixel value
     *
     * @param unit
     *   unit value to convert into pixel
     * @returns
     *   width pixel value of unit
     */
    int u_to_px_w(float unit) const;

    /**
     * convert height unit to pixel value
     *
     * @param unit
     *   unit value to convert into pixel
     * @returns
     *   height pixel value of unit
     */
    int u_to_px_h(float unit) const;

    /**
     * convert width pixel to unit value
     *
     * @param pixel
     *   pixel value to convert into unit
     * @returns
     *   width unit value of pixel
     */
    float px_to_u_w(int pixel) const;

    /**
     * convert height pixel to unit value
     *
     * @param pixel
     *   pixel value to convert into unit
     * @returns
     *   height unit value of pixel
     */
    float px_to_u_h(int pixel) const;

    /**
     * recompute values in config given the current state
     *
     * @param model
     *   applications model
     */
    void recompute_config(const Model *model);

    /**
     * get the width of the player in pixels
     *
     * @returns
     *   width of the player in pixels
     */
    int get_player_width_px() const;
};
