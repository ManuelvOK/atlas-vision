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

template<typename S, typename J>
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
    virtual void init() {
        this->_interface_model->init_colors(this->_simulation_model->_jobs.size());

        std::map<unsigned, std::vector<unsigned>> submissions;
        unsigned max_submissions = 0;
        std::map<unsigned, std::vector<unsigned>> deadlines;
        unsigned max_deadlines = 0;

        std::tie(submissions, max_submissions) = submissions_from_jobs(this->_simulation_model->_jobs);
        std::tie(deadlines, max_deadlines) = deadlines_from_jobs(this->_simulation_model->_jobs);

        this->init_deadline_rect(submissions, deadlines);
        this->create_submission_drawables(submissions);
        this->create_deadline_drawables(deadlines);

        this->init_cores_rect();
        this->create_schedule_drawables();

        this->init_message_rect();
        this->create_message_drawables(this->_simulation_model->_messages);
        this->create_legend(this->_simulation_model->_jobs);

        this->_simulation_model->_dirty = false;
    }

    virtual void reset() {
        for (unsigned i = 0; i < this->_simulation_model->_n_cores; ++i) {
            std::stringstream rect_name;
            rect_name << "core-" << i;
            SDL_GUI::Drawable *core_rect =
                this->_default_interface_model->find_first_drawable(rect_name.str());
            if (core_rect) {
                core_rect->remove_children([](SDL_GUI::Drawable *d){
                    return d->_type != "Rect";
                });
            }
        }

        for (std::string id: {"deadline", "messages", "legend"}) {
            this->_default_interface_model->find_first_drawable(id)->remove_all_children();
        }
    }

    void reinit() {
        this->reset();
        this->init();
    }

    /**
     * Get all the submissions there are from a list of jobs grouped by submission time
     * @param jobs list of jobs to get submissions for
     * @return job submissions grouped by submission time
     */
    std::tuple<std::map<unsigned, std::vector<unsigned>>, unsigned>
    submissions_from_jobs(std::vector<J *> jobs) const {
        std::map<unsigned, std::vector<unsigned>> submissions;
        size_t max_submissions = 0;
        for (const J *job: jobs) {
            submissions[job->_submission_time].push_back(job->_id);
            max_submissions =
                std::max(max_submissions, submissions[job->_submission_time].size());
        }
        return std::make_tuple(submissions, max_submissions);
    }

    /**
     * Get all the deadlines there are from a list of jobs grouped by deadline time
     * @param jobs list of jobs to get deadlines for
     * @return job deadlines grouped by deadline time
     */
    std::tuple<std::map<unsigned, std::vector<unsigned>>, unsigned>
    deadlines_from_jobs(std::vector<J *> jobs) const {
        std::map<unsigned, std::vector<unsigned>> deadlines;
        size_t max_deadlines = 0;
        for (const J *job: jobs) {
            deadlines[job->_deadline].push_back(job->_id);
            max_deadlines = std::max(max_deadlines, deadlines[job->_deadline].size());
        }
        return std::make_tuple(deadlines, max_deadlines);
    }

    /**
     * Create Drawables for all the submissions (Submission Arrows that is)
     * @param submissions job submissions ordered by submission time
     * @param deadline_rect parent drawable to add submissions as childs to
     */
    void create_submission_drawables(std::map<unsigned, std::vector<unsigned>> submissions) {
        SDL_GUI::Drawable *deadline_rect =
            this->_default_interface_model->find_first_drawable("deadline");
        const unsigned arrow_height = Arrow::height(Arrow::Direction::UP);

        for (const auto &[timestamp, subs]: submissions) {
            const int distance_height = (subs.size() - 1) * interface_config.player.arrow_distance_px;
            int offset = deadline_rect->height() - arrow_height - distance_height;
            for (unsigned job_id: subs) {
                const J *job = this->_simulation_model->_jobs[job_id];
                /* TODO: get rid of magic number */
                SDL_GUI::Position position(10 + this->_interface_model->px_width(timestamp), offset);
                JobArrow *a = new JobArrow(job, timestamp, this->_interface_model, this->_simulation_model,
                                        position);
                this->_simulation_model->_drawables_jobs[a].insert(job->_id);
                deadline_rect->add_child(a);
                offset += interface_config.player.arrow_distance_px;
            }
        }
    }

    /**
     * Create Drawables for all the deadlines (Deadline Arrows that is)
     * @param deadlines job deadlines ordered by deadlines time
     * @param deadline_rect parent drawable to add deadlines as childs to
     */
    void create_deadline_drawables(std::map<unsigned, std::vector<unsigned>> deadlines) {
        SDL_GUI::Drawable *deadline_rect =
            this->_default_interface_model->find_first_drawable("deadline");

        for (const auto &[timestamp, dls]: deadlines) {
            const int distance_height = (dls.size() - 1) * interface_config.player.arrow_distance_px;
            int offset = distance_height;
            for (unsigned job_id: dls) {
                const J *job = this->_simulation_model->_jobs[job_id];
                /* TODO: get rid of magic number */
                SDL_GUI::Position position(10 + this->_interface_model->px_width(timestamp), offset);
                JobArrow *a = new JobArrow(job, timestamp, this->_interface_model, this->_simulation_model,
                                        position, Arrow::Direction::DOWN);
                this->_simulation_model->_drawables_jobs[a].insert(job->_id);
                deadline_rect->add_child(a);
                offset -= interface_config.player.arrow_distance_px;
            }
        }
    }

    /**
     * calculate and set the height of the deadline rect
     * @param submissions job submissions ordered by submission time
     * @param deadlines job deadlines ordered by deadlines time
     */
    void init_deadline_rect(std::map<unsigned, std::vector<unsigned>> submissions,
                            std::map<unsigned, std::vector<unsigned>> deadlines) {
        std::map<unsigned, int> heights;

        int sub_height = Arrow::height(Arrow::Direction::UP);

        /* submissions (map is empty yet) */
        for (auto const &[timestamp, subs]: submissions) {
            /* height of first arrow */
            heights[timestamp] = sub_height;

            /* height of other arrows (overlapping) */
            heights.at(timestamp) += (subs.size() - 1) * interface_config.player.arrow_distance_px;
        }

        unsigned dl_height = Arrow::height(Arrow::Direction::DOWN);

        /* deadlines (map is already filled) */
        for (auto const &[timestamp, dls]: deadlines) {
            if (not heights.contains(timestamp)) {
                heights[timestamp] = 0;
            } else {
                heights.at(timestamp) += interface_config.player.arrow_distance_px;
            }

            /* height of first arrow */
            heights.at(timestamp) += dl_height;

            /* height of other arrows (overlapping) */
            heights.at(timestamp) += (dls.size() - 1) * interface_config.player.arrow_distance_px;
        }

        int max_height = std::max_element(heights.begin(), heights.end(),
                            [](auto a, auto b){return a.second < b.second;})->second;

        SDL_GUI::Drawable *deadline_rect =
            this->_default_interface_model->find_first_drawable("deadline");
        deadline_rect->set_height(max_height);
    }

    /**
     * Create drawables for all schedules and register callbacks to reposition/hide them on occasion.
     */
    virtual void create_schedule_drawables() = 0;

    /**
     * set the Style of the scheduler as given in the config
     */
    virtual void init_cores_rect() = 0;

    /** create callback to change input state on mouseover */
    void init_message_rect() {
        SDL_GUI::Drawable *message_rect =
            this->_default_interface_model->find_first_drawable("messages");
        message_rect->enable_scrolling_y();
        message_rect->set_enforcing_limits(true);
        message_rect->add_recalculation_callback([this](SDL_GUI::Drawable *d) {
                SDL_GUI::Position mouse_position = this->_input_model->mouse_position();
                if (d->is_inside(mouse_position)) {
                    if (this->_input_model->state() != InputState::IN_MESSAGES) {
                        this->_input_model->set_state(InputState::IN_MESSAGES, false);
                    }
                } else if (this->_input_model->state() == InputState::IN_MESSAGES) {
                    this->_input_model->set_state(InputState::ALL, false);
                }
            });
    }

    /**
    * create drawables for all the event messages
    * @param messages list of messages to create drawables for
    */
    void create_message_drawables(std::vector<Message *> messages) {
        SDL_GUI::Drawable *message_rect =
            this->_default_interface_model->find_first_drawable("messages");
        int offset = 5;
        for (Message *message: messages) {
            std::stringstream ss;
            ss << message->_timestamp << ": " << message->_message;
            MessageText *t = new MessageText(ss.str(), this->_default_interface_model->font(),
                                            message_rect->width() - 10, {5, offset});
            t->add_recalculation_callback([this, message, t](SDL_GUI::Drawable *) {
                if (this->_player_model->_position >= message->_timestamp) {
                    t->activate();
                } else {
                    t->deactivate();
                }
            });
            this->_simulation_model->_drawables_jobs[t] = message->_jobs;
            this->_simulation_model->_drawables_messages[t] = message;
            message_rect->add_child(t);

            offset += t->height() + 2;

            SDL_GUI::Line *l = new SDL_GUI::Line(SDL_GUI::Position(5, offset - 3),
                                                SDL_GUI::Position(t->width(), offset - 3));
            l->hide();
            this->_simulation_model->_messages_underlines[message] = l;
            message_rect->add_child(l);
        }
        int overflow = offset - message_rect->height();

        message_rect->set_min_limit({0, -std::max(0, overflow)});
    }

    /**
    * create all the drawables for the visual color to job number mapping
    */
    virtual void create_legend(std::vector<J *> jobs) {
        SDL_GUI::Drawable *legend_rect = this->_default_interface_model->find_first_drawable("legend");

        SDL_GUI::Position position(15, 15);
        for (unsigned i = 0; i < jobs.size(); ++i) {
            const J *job = jobs[i];
            SDL_GUI::Drawable *info = create_job_information(job);
            info->set_position(position);
            legend_rect->add_child(info);

            position._x += info->width() + 10;

            if (position._x + info->width() > legend_rect->width() - 30) {
                position._x = 15;
                position._y += info->height() + 10;
            }
        }
    }

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

    virtual void update() override {
        if (this->_input_model->is_pressed(InputKey::QUIT)) {
            this->_application->_is_running = false;
        }
        if (this->_input_model->is_down(InputKey::REINIT)) {
            this->_simulation_model->_dirty = true;
        }

        if (this->_simulation_model->_dirty) {
            this->reinit();
        }

        SDL_GUI::Position mouse_position = this->_input_model->mouse_position();
        std::vector<SDL_GUI::Drawable *>hovered =
            this->_default_interface_model->find_drawables_at_position(mouse_position);

        /* highlight jobs */
        std::map<const SDL_GUI::Drawable *, std::set<unsigned>> drawables_jobs =
            this->_simulation_model->_drawables_jobs;
        auto first_hovered = std::find_if(hovered.begin(), hovered.end(),
            [drawables_jobs, mouse_position](SDL_GUI::Drawable *d) {
                return drawables_jobs.contains(d)
                    and not d->is_hidden()
                    and d->is_inside_clip_rect(mouse_position);
            });

        this->_simulation_model->_highlighted_jobs.clear();
        if (first_hovered != hovered.end()) {
            std::set<unsigned> ids = this->_simulation_model->_drawables_jobs[*first_hovered];
            this->_simulation_model->_highlighted_jobs = ids;
        }

        /* clear underline for hovered message */
        if (this->_simulation_model->_hovered_message) {
            this->_simulation_model->_messages_underlines[this->_simulation_model->_hovered_message]->hide();
        }

        if (this->_input_model->state() == InputState::IN_MESSAGES) {
            /* hovered message */
            this->_simulation_model->_hovered_message = nullptr;
            std::map<const SDL_GUI::Drawable *, Message *> drawables_messages =
                this->_simulation_model->_drawables_messages;
            first_hovered = std::find_if(hovered.begin(), hovered.end(),
                [drawables_messages, mouse_position](SDL_GUI::Drawable *d) {
                    return drawables_messages.contains(d)
                        and not d->is_hidden()
                        and d->is_inside_clip_rect(mouse_position);
                });
            if (first_hovered != hovered.end()) {
                this->_simulation_model->_hovered_message =
                    this->_simulation_model->_drawables_messages[*first_hovered];
                this->_simulation_model->_messages_underlines[this->_simulation_model->_hovered_message]->show();
            }
            /* scrolling */
            SDL_GUI::Drawable *message_rect =
                this->_default_interface_model->find_first_drawable("messages");
            if (this->_input_model->mouse_wheel()._y > 0) {
                message_rect->scroll_down(40);
            }

            if (this->_input_model->mouse_wheel()._y < 0) {
                message_rect->scroll_up(40);
            }
        }

        /* draw debug messages */
        SDL_GUI::Drawable *debug_rect =
            this->_default_interface_model->find_first_drawable("debug");
        debug_rect->remove_all_children();

        int offset = 5;
        for (std::string message: this->_simulation_model->_debug_messages) {
            MessageText *t = new MessageText(message, this->_default_interface_model->font(),
                                            debug_rect->width() - 10, {5, offset});
            offset += t->height() + 2;
            debug_rect->add_child(t);
        }
        this->_simulation_model->_debug_messages.clear();

    }

};
