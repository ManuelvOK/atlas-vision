#pragma once


#include <controllers/simulation_view_controller.h>
#include <models/atlas_model.h>

/** Controller for everything related to the scheduler */
class AtlasViewController : public SimulationViewController {
    AtlasModel *_atlas_model;                           /**< Model for the scheduler */


    /**
     * initialise the controller. This is called from the constructor and creates all the drawables
     * for the scheduler with their recalculation callbacks.
     */
    void init() override;

    void reset() override;

    /**
     * Create drawables for all schedules and register callbacks to reposition/hide them on occasion.
     */
    void create_schedule_drawables() override;

    /**
     * set the Style of the scheduler as given in the config
     */
    void init_cores_rect() override;

    /**
    * Create drawables for all visibilities
    * @param early_cfs_schedules list of schedules to add visibility drawables for
    */
    void create_CFS_visibility_drawables(std::vector<EarlyCfsSchedule *> early_cfs_schedules);


    /**
    * create all the drawables for the dependency graph of the jobs
    * @param jobs jobs to draw dependency graph for
    */
    void create_dependency_graph(std::vector<Job *> jobs);

    SDL_GUI::Drawable *create_job_information(const Job *job) override;

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
    AtlasViewController(SDL_GUI::ApplicationBase *application, AtlasModel *atlas_model,
                        InterfaceModel *interface_model,
                        SDL_GUI::InterfaceModel *default_interface_model,
                        InputModel *input_model,
                        const PlayerModel *player_model);
};
