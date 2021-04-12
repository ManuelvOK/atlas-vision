#pragma once

#include <SDL_GUI/application.h>
#include <SDL_GUI/controllers/controller_base.h>
#include <SDL_GUI/models/interface_model.h>

#include <models/atlas_model.h>
#include <models/interface_model.h>
#include <models/input_model.h>
#include <models/player_model.h>

/** Controller for everything related to the scheduler */
class AtlasController : public SDL_GUI::ControllerBase {
    SDL_GUI::ApplicationBase *_application;             /**< The application */
    AtlasModel *_atlas_model;                           /**< Model for the scheduler */
    InterfaceModel *_interface_model;                   /**< The Applications interface model */
    SDL_GUI::InterfaceModel *_default_interface_model;  /**< The interface model from the library */
    InputModel *_input_model;  /**< The applications input model */
    const PlayerModel *_player_model;                   /**< Model for the schedule player */


    /**
     * initialise the controller. This is called from the constructor and creates all the drawables
     * for the scheduler with their recalculation callbacks.
     */
    void init();

    void reinit();

    /**
     * Get all the submissions there are from a list of jobs grouped by submission time
     * @param jobs list of jobs to get submissions for
     * @return job submissions grouped by submission time
     */
    std::tuple<std::map<int, std::vector<int>>, int>
    submissions_from_jobs(std::vector<Job *> jobs) const;

    /**
     * Get all the deadlines there are from a list of jobs grouped by deadline time
     * @param jobs list of jobs to get deadlines for
     * @return job deadlines grouped by deadline time
     */
    std::tuple<std::map<int, std::vector<int>>, int>
    deadlines_from_jobs(std::vector<Job *> jobs) const;

    /**
     * Create Drawables for all the submissions (Submission Arrows that is)
     * @param submissions job submissions ordered by submission time
     * @param deadline_rect parent drawable to add submissions as childs to
     */
    void create_submission_drawables(std::map<int, std::vector<int>> submissions);

    /**
     * Create Drawables for all the deadlines (Deadline Arrows that is)
     * @param deadlines job deadlines ordered by deadlines time
     * @param deadline_rect parent drawable to add deadlines as childs to
     */
    void create_deadline_drawables(std::map<int, std::vector<int>> deadlines);

    /**
     * calculate and set the height of the deadline rect
     * @param submissions job submissions ordered by submission time
     * @param deadlines job deadlines ordered by deadlines time
     */
    void init_deadline_rect(std::map<int, std::vector<int>> submissions,
                            std::map<int, std::vector<int>> deadlines);

    /**
     * Create drawables for all schedules and register callbacks to reposition/hide them on occasion.
     */
    void create_schedule_drawables();

    /**
     * set the Style of the scheduler as given in the config
     */
    void init_cores_rect();

    /**
    * Create drawables for all visibilities
    * @param early_cfs_schedules list of schedules to add visibility drawables for
    */
    void create_CFS_visibility_drawables(std::vector<EarlyCfsSchedule *> early_cfs_schedules);

    /** create callback to change input state on mouseover */
    void init_message_rect();

    /**
    * create drawables for all the event messages
    * @param messages list of messages to create drawables for
    */
    void create_message_drawables(std::vector<Message *> messages);

    /**
    * create all the drawables for the dependency graph of the jobs
    * @param jobs jobs to draw dependency graph for
    */
    void create_dependency_graph(std::vector<Job *> jobs);

    /**
    * create all the drawables for the visual color to job number mapping
    */
    void create_legend(std::vector<Job *> jobs);

    /**
    * Create the Drawable that holds all information for one Job
    * @param job The job to create the drawable for
    * @return Jobs information drawable for the legend
    */
    SDL_GUI::Drawable *create_job_information(const Job *job);

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
                    InputModel *input_model,
                    const PlayerModel *player_model);

    void update() override;
};
