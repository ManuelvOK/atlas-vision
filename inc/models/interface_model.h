#pragma once

#include <vector>

#include <SDL_GUI/models/model_base.h>
#include <SDL_GUI/gui/rgb.h>

#include <models/player_model.h>

#include <config/interface_config.h>

/**
 * Model for more interface related data.
 * TODO: this should inherit from SDL_GUI::InterfaceModel.
 */
enum class SchedulerType;
class InterfaceModel : public SDL_GUI::ModelBase {
public:
    /**
     * Constructor
     * @param player_model the applications playermodel
     */
    InterfaceModel(PlayerModel *player_model) : _player_model(player_model),
                                                _unit_height(interface_config.unit.height_px) {}

    PlayerModel *_player_model;     /**< the applications player model */
    float _unit_height;             /**< current unit to pixel ratio of height */
    bool _rescaled = false;         /**< flag determining rescaling */
    std::vector<unsigned> _colors;  /**< job colors */

    /**
     * initialise color presets
     * @param n_jobs number of jobs to calculate color preset for
     */
    void init_colors(int n_jobs);

    /**
     * get rgb values of the color for a given job
     * @param job number of job to get color for
     * @param modifier color alpha
     * @return RGB color object of job color
     */
    SDL_GUI::RGB get_color(int job, float modifier = 1.0) const;

    /**
     * convert width unit to pixel value
     * @param unit unit value to convert into pixel
     * @return width pixel value of unit
     */
    int px_width(float unit) const;

    /**
     * convert height unit to pixel value
     * @param unit unit value to convert into pixel
     * @return height pixel value of unit
     */
    int px_height(float unit) const;

    /**
     * convert width pixel to unit value
     * @param pixel pixel value to convert into unit
     * @return width unit value of pixel
     */
    float unit_width(int pixel) const;

    /**
     * convert height pixel to unit value
     * @param pixel pixel value to convert into unit
     * @return height unit value of pixel
     */
    float unit_height(int pixel) const;

    /**
     * get the width of the player in pixels
     * @return width of the player in pixels
     */
    int get_player_width_px() const;

    /**
     * get the y-offset of the given scheduler inside the core rect
     * @return schedulers offset insode core rect
     */
    int scheduler_offset(SchedulerType t) const;

    /**
     * get the height of a single core rect
     * @return height of a core rect
     */
    int core_rect_height() const;
};
