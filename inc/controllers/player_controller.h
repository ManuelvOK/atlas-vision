#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>
#include <SDL_GUI/inc/models/interface_model.h>

#include <input_keys.h>
#include <models/atlas_model.h>
#include <models/input_model.h>
#include <models/interface_model.h>
#include <models/player_model.h>


/** controller for drawing a certain position in the schedule. */
class PlayerController : public SDL_GUI::ControllerBase {
    PlayerModel *_player_model;         /**< Model for the player */
    InputModel *_input_model;           /**< The applications input model */
    InterfaceModel *_interface_model;   /**< The applications interface model */

    /** the interface model from the SDL_GUI library */
    SDL_GUI::InterfaceModel *_default_interface_model;

    /** flag that determines whether the cursor line is being dragged right now */
    bool _dragging = false;

    /**
     * initialise this controller. This gets called from the constructor.
     * @param atlas_model the applications atlas model
     */
    void init(const AtlasModel *atlas_model);

    /** change state on input. This gets called from update() */
    void evaluate_input();

    /** change position on mouse movement if _dragging is true */
    void drag();
public:
    /**
     * Constructor
     * @param player_model model for the player
     * @param input_model the applications input model
     * @param atlas_model the applications atlas model
     * @param interface_model the applications interface model
     * @param default_interface_model interface model from the SDL_GUI library
     */
    PlayerController(PlayerModel *player_model, InputModel *input_model,
                     const AtlasModel *atlas_model, InterfaceModel *interface_model,
                     SDL_GUI::InterfaceModel *default_interface_model);

    void update() override;
};
