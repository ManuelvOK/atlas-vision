#pragma once

#include <SDL_GUI/inc/application.h>
#include <SDL_GUI/inc/controllers/controller_base.h>
#include <SDL_GUI/inc/models/input_model.h>
#include <SDL_GUI/inc/models/interface_model.h>

#include <input_keys.h>
#include <models/atlas_model.h>
#include <models/interface_model.h>
#include <models/player_model.h>

/** Controller for everything related to the scheduler */
class AtlasController : public SDL_GUI::ControllerBase {
    SDL_GUI::ApplicationBase *_application;             /**< The application */
    AtlasModel *_atlas_model;                           /**< Model for the scheduler */
    InterfaceModel *_interface_model;                   /**< The Applications interface model */
    SDL_GUI::InterfaceModel *_default_interface_model;  /**< The interface model from the library */
    const SDL_GUI::InputModel<InputKey> *_input_model;  /**< The applications input model */
    const PlayerModel *_player_model;                   /**< Model for the schedule player */


    /**
     * initialise the controller. This is called from the constructor and creates all the drawables
     * for the scheduler with their recalculation callbacks.
     */
    void init();
public:
    /**
     * Constructor
     * @param application The application
     * @param atlas_model model for the scheduler
     * @param interface_model The applications interface model
     * @param default_interface_model The interface model from the SDL_GUI library
     * @param input_model the applications input model
     * @param player_model Model for the schedule player
     */
    AtlasController(SDL_GUI::ApplicationBase *application, AtlasModel *atlas_model,
                    InterfaceModel *interface_model,
                    SDL_GUI::InterfaceModel *default_interface_model,
                    const SDL_GUI::InputModel<InputKey> *input_model,
                    const PlayerModel *player_model);

    void update() override;
};
