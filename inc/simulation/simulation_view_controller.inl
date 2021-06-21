#pragma once
#include <simulation/simulation_view_controller.h>

template <typename S, typename J>
inline void SimulationViewController<S, J>::init() {
    this->_interface_model->init_colors(this->_simulation_model->_jobs.size());

    std::map<int, std::vector<unsigned>> submissions;
    unsigned max_submissions = 0;
    std::map<int, std::vector<unsigned>> deadlines;
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

template <typename S, typename J>
inline void SimulationViewController<S, J>::reset() {
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

template <typename S, typename J>
inline void SimulationViewController<S, J>::reinit() {
    this->reset();
    this->init();
}

template <typename S, typename J>
inline std::tuple<std::map<int, std::vector<unsigned>>, unsigned>
SimulationViewController<S, J>::submissions_from_jobs(std::vector<J *> jobs) const {
    std::map<int, std::vector<unsigned>> submissions;
    size_t max_submissions = 0;
    for (const J *job: jobs) {
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions =
            std::max(max_submissions, submissions[job->_submission_time].size());
    }
    return std::make_tuple(submissions, max_submissions);
}

template <typename S, typename J>
inline std::tuple<std::map<int, std::vector<unsigned>>, unsigned>
SimulationViewController<S, J>::deadlines_from_jobs(std::vector<J *> jobs) const {
    std::map<int, std::vector<unsigned>> deadlines;
    size_t max_deadlines = 0;
    for (const J *job: jobs) {
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, deadlines[job->_deadline].size());
    }
    return std::make_tuple(deadlines, max_deadlines);
}

template <typename S, typename J>
inline std::vector<JobArrow *>
SimulationViewController<S, J>::create_submission_drawables(std::map<int,
                                                            std::vector<unsigned>> submissions) {
    std::vector<JobArrow *> arrows;
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
            arrows.push_back(a);
            deadline_rect->add_child(a);
            offset += interface_config.player.arrow_distance_px;
        }
    }
    return arrows;
}

template <typename S, typename J>
inline std::vector<JobArrow *>
SimulationViewController<S, J>::create_deadline_drawables(std::map<int,
                                                          std::vector<unsigned>> deadlines) {
    std::vector<JobArrow *> arrows;
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
            arrows.push_back(a);
            deadline_rect->add_child(a);
            offset -= interface_config.player.arrow_distance_px;
        }
    }
    return arrows;
}

template <typename S, typename J>
inline void
SimulationViewController<S, J>::init_deadline_rect(std::map<int,
                                                   std::vector<unsigned>> submissions,
                                                   std::map<int,
                                                            std::vector<unsigned>> deadlines) {
    std::map<int, int> heights;

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


template <typename S, typename J>
inline void
SimulationViewController<S, J>::init_message_rect() {
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

template <typename S, typename J>
inline void
SimulationViewController<S, J>::create_message_drawables(std::vector<Message *> messages) {
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

template <typename S, typename J>
inline void
SimulationViewController<S, J>::create_legend(std::vector<J *> jobs) {
    SDL_GUI::Drawable *legend_rect = this->_default_interface_model->find_first_drawable("legend");

    SDL_GUI::Position position(15, 15);
    for (unsigned i = 0; i < jobs.size(); ++i) {
        const J *job = jobs[i];
        SDL_GUI::Drawable *info = this->create_job_information(job);
        info->set_position(position);
        legend_rect->add_child(info);

        position._x += info->width() + 10;

        if (position._x + info->width() > legend_rect->width() - 30) {
            position._x = 15;
            position._y += info->height() + 10;
        }
    }
}

template <typename S, typename J>
inline void
SimulationViewController<S, J>::update() {
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
