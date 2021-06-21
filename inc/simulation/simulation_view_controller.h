#pragma once

#include <sstream>


#include <SDL_GUI/application.h>
#include <SDL_GUI/gui/primitives/line.h>
#include <SDL_GUI/controllers/controller_base.h>
#include <SDL_GUI/models/interface_model.h>

#include <gui/interface_model.h>
#include <gui/job_arrow.h>
#include <gui/message_text.h>
#include <input/input_model.h>
#include <player/player_model.h>
#include <simulation/simulation_model.h>

template <typename S, typename J>
class SimulationViewController : public SDL_GUI::ControllerBase {
protected:
    SDL_GUI::ApplicationBase *_application;             /**< The application */
    SimulationModel<S, J> *_simulation_model;           /**< Model for the scheduler */
    InterfaceModel *_interface_model;                   /**< The Applications interface model */
    SDL_GUI::InterfaceModel *_default_interface_model;  /**< The interface model from the library */
    InputModel *_input_model;                           /**< The applications input model */
    const PlayerModel *_player_model;                   /**< Model for the schedule player */

    /**
     * initialise the controller. This is called from the constructor and creates all the drawables
     * for the scheduler with their recalculation callbacks.
     */
    virtual void init();

    virtual void reset();

    void reinit();

    /**
     * Get all the submissions there are from a list of jobs grouped by submission time
     * @param jobs list of jobs to get submissions for
     * @return job submissions grouped by submission time
     */
    std::tuple<std::map<int, std::vector<unsigned>>, unsigned>
    submissions_from_jobs(std::vector<J *> jobs) const;

    /**
     * Get all the deadlines there are from a list of jobs grouped by deadline time
     * @param jobs list of jobs to get deadlines for
     * @return job deadlines grouped by deadline time
     */
    std::tuple<std::map<int, std::vector<unsigned>>, unsigned>
    deadlines_from_jobs(std::vector<J *> jobs) const;

    /**
     * Create Drawables for all the submissions (Submission Arrows that is)
     * @param submissions job submissions ordered by submission time
     * @param deadline_rect parent drawable to add submissions as childs to
     */
    virtual std::vector<JobArrow *>
    create_submission_drawables(std::map<int, std::vector<unsigned>> submissions);

    /**
     * Create Drawables for all the deadlines (Deadline Arrows that is)
     * @param deadlines job deadlines ordered by deadlines time
     * @param deadline_rect parent drawable to add deadlines as childs to
     */
    virtual std::vector<JobArrow *>
    create_deadline_drawables(std::map<int, std::vector<unsigned>> deadlines);

    /**
     * calculate and set the height of the deadline rect
     * @param submissions job submissions ordered by submission time
     * @param deadlines job deadlines ordered by deadlines time
     */
    void init_deadline_rect(std::map<int, std::vector<unsigned>> submissions,
                            std::map<int, std::vector<unsigned>> deadlines);

    /**
     * Create drawables for all schedules and register callbacks to reposition/hide them on occasion.
     */
    virtual void create_schedule_drawables() = 0;

    /**
     * set the Style of the scheduler as given in the config
     */
    virtual void init_cores_rect() = 0;

    /** create callback to change input state on mouseover */
    void init_message_rect();

    /**
    * create drawables for all the event messages
    * @param messages list of messages to create drawables for
    */
    void create_message_drawables(std::vector<Message *> messages);

    /**
    * create all the drawables for the visual color to job number mapping
    */
    virtual void create_legend(std::vector<J *> jobs);

    /**
    * Create the Drawable that holds all information for one Job
    * @param job The job to create the drawable for
    * @return Jobs information drawable for the legend
    */
    virtual SDL_GUI::Drawable *create_job_information(const J *job) = 0;

public:
    SimulationViewController(SDL_GUI::ApplicationBase *application,
                             SimulationModel<S, J> *simulation_model,
                             InterfaceModel *interface_model,
                             SDL_GUI::InterfaceModel *default_interface_model,
                             InputModel *input_model,
                             const PlayerModel *player_model)
        : _application(application),
          _simulation_model(simulation_model),
          _interface_model(interface_model),
          _default_interface_model(default_interface_model),
          _input_model(input_model),
          _player_model(player_model) {}

    virtual void update() override;

};

#include <simulation/simulation_view_controller.inl>
