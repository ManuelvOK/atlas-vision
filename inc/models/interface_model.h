#pragma once

#include <SDL_GUI/inc/models/interface_model.h>

#include <SDL_GUI/inc/gui/rgb.h>

class InterfaceModel : public SDL_GUI::InterfaceModel {
public:
    InterfaceModel();

    float _unit_width;       /**< current unit to pixel ratio of width*/
    float _unit_width_min;   /**< minimal unit to pixel ratio of width */
    float _unit_height;       /**< current unit to pixel ratio of height */

    bool _rescaled = false;  /**< flag determining rescaling */

    std::vector<unsigned> _colors;   /**< job colors */

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
