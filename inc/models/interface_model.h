#pragma once

#include <vector>

#include <SDL_GUI/inc/models/model_base.h>
#include <SDL_GUI/inc/gui/rgb.h>

#include <models/player_model.h>

#include <config/interface_config.h>

class InterfaceModel : public SDL_GUI::ModelBase {
public:
    InterfaceModel(PlayerModel *player_model) : _player_model(player_model),
                                                _unit_height(interface_config.unit.height_px) {}

    PlayerModel *_player_model;
    float _unit_height;       /**< current unit to pixel ratio of height */

    bool _rescaled = false;  /**< flag determining rescaling */

    std::vector<unsigned> _colors;   /**< job colors */

    /**
     * initialise color presets
     *
     * @param n_jobs
     *   number of jobs to calculate color preset for
     */
    void init_colors(int n_jobs);

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
    SDL_GUI::RGB get_color(int job, float modifier = 1.0) const;

    /**
     * convert width unit to pixel value
     *
     * @param unit
     *   unit value to convert into pixel
     * @returns
     *   width pixel value of unit
     */
    int px_width(float unit) const;

    /**
     * convert height unit to pixel value
     *
     * @param unit
     *   unit value to convert into pixel
     * @returns
     *   height pixel value of unit
     */
    int px_height(float unit) const;

    /**
     * convert width pixel to unit value
     *
     * @param pixel
     *   pixel value to convert into unit
     * @returns
     *   width unit value of pixel
     */
    float unit_width(int pixel) const;

    /**
     * convert height pixel to unit value
     *
     * @param pixel
     *   pixel value to convert into unit
     * @returns
     *   height unit value of pixel
     */
    float unit_height(int pixel) const;

    /**
     * get the width of the player in pixels
     *
     * @returns
     *   width of the player in pixels
     */
    int get_player_width_px() const;
};
