#include <controllers/atlas_controller.h>

#include <algorithm>
#include <limits>
#include <sstream>

#include <SDL_GUI/gui/drawable.h>
#include <SDL_GUI/gui/rgb.h>

#include <config/interface_config.h>
#include <gui/job_arrow.h>
#include <gui/job_rect.h>
#include <gui/message_text.h>
#include <gui/schedule_rect.h>
#include <gui/visibility_line.h>

AtlasController::AtlasController(SDL_GUI::ApplicationBase *application, AtlasModel *atlas_model,
                                 InterfaceModel *interface_model,
                                 SDL_GUI::InterfaceModel *default_interface_model,
                                 InputModel *input_model,
                                 const PlayerModel *player_model) :
    _application(application),
    _atlas_model(atlas_model),
    _interface_model(interface_model),
    _default_interface_model(default_interface_model),
    _input_model(input_model),
    _player_model(player_model) {
}

void AtlasController::init() {
    this->_interface_model->init_colors(this->_atlas_model->_jobs.size());

    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;

    std::tie(submissions, max_submissions) = submissions_from_jobs(this->_atlas_model->_jobs);
    std::tie(deadlines, max_deadlines) = deadlines_from_jobs(this->_atlas_model->_jobs);

    this->init_deadline_rect(submissions, deadlines);
    this->create_submission_drawables(submissions);
    this->create_deadline_drawables(deadlines);

    this->init_cores_rect();
    this->create_schedule_drawables();

    this->create_CFS_visibility_drawables(this->_atlas_model->_early_cfs_schedules);

    this->init_message_rect();
    this->create_message_drawables(this->_atlas_model->_messages);
    this->create_dependency_graph(this->_atlas_model->_jobs);
    this->create_legend(this->_atlas_model->_jobs);

    this->_atlas_model->_dirty = false;
}

void AtlasController::reinit() {
    std::cout << "reinitialising atlas_controller" << std::endl;
    for (int i = 0; i < this->_atlas_model->_n_cores; ++i) {
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

    for (std::string id: {"deadline", "messages", "dependencies", "legend"}) {
        this->_default_interface_model->find_first_drawable(id)->remove_all_children();
    }

    this->init();
}


void AtlasController::update() {
    if (this->_input_model->is_pressed(InputKey::QUIT)) {
        this->_application->_is_running = false;
    }
    if (this->_input_model->is_down(InputKey::REINIT)) {
        this->_atlas_model->_dirty = true;
    }

    if (this->_atlas_model->_dirty) {
        this->reinit();
    }
    SDL_GUI::Position mouse_position = this->_input_model->mouse_position();
    std::vector<SDL_GUI::Drawable *>hovered =
        this->_default_interface_model->find_drawables_at_position(mouse_position);

    /* highlight jobs */
    std::map<const SDL_GUI::Drawable *, std::set<int>> drawables_jobs =
        this->_atlas_model->_drawables_jobs;
    auto first_hovered = std::find_if(hovered.begin(), hovered.end(),
        [drawables_jobs, mouse_position](SDL_GUI::Drawable *d) {
            return drawables_jobs.contains(d)
                   and not d->is_hidden()
                   and d->is_inside_clip_rect(mouse_position);
        });

    this->_atlas_model->_highlighted_jobs.clear();
    if (first_hovered != hovered.end()) {
        std::set<int> ids = this->_atlas_model->_drawables_jobs[*first_hovered];
        this->_atlas_model->_highlighted_jobs = ids;
    }

    /* clear underline for hovered message */
    if (this->_atlas_model->_hovered_message) {
        this->_atlas_model->_messages_underlines[this->_atlas_model->_hovered_message]->hide();
    }

    if (this->_input_model->state() == InputState::IN_MESSAGES) {
        /* hovered message */
        this->_atlas_model->_hovered_message = nullptr;
        std::map<const SDL_GUI::Drawable *, Message *> drawables_messages =
            this->_atlas_model->_drawables_messages;
        first_hovered = std::find_if(hovered.begin(), hovered.end(),
            [drawables_messages, mouse_position](SDL_GUI::Drawable *d) {
                return drawables_messages.contains(d)
                    and not d->is_hidden()
                    and d->is_inside_clip_rect(mouse_position);
            });
        if (first_hovered != hovered.end()) {
            this->_atlas_model->_hovered_message =
                this->_atlas_model->_drawables_messages[*first_hovered];
            this->_atlas_model->_messages_underlines[this->_atlas_model->_hovered_message]->show();
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
    for (std::string message: this->_atlas_model->_debug_messages) {
        MessageText *t = new MessageText(message, this->_default_interface_model->font(),
                                         debug_rect->width() - 10, {5, offset});
        offset += t->height() + 2;
        debug_rect->add_child(t);
    }
    this->_atlas_model->_debug_messages.clear();

}

std::tuple<std::map<int, std::vector<int>>, int>
AtlasController::submissions_from_jobs(std::vector<Job *> jobs) const {
    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    for (const Job *job: jobs) {
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions =
            std::max(max_submissions, static_cast<int>(submissions[job->_submission_time].size()));
    }
    return std::make_tuple(submissions, max_submissions);
}


std::tuple<std::map<int, std::vector<int>>, int>
AtlasController::deadlines_from_jobs(std::vector<Job *> jobs) const {
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;
    for (const Job *job: jobs) {
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, static_cast<int>(deadlines[job->_deadline].size()));
    }
    return std::make_tuple(deadlines, max_deadlines);
}

void AtlasController::create_submission_drawables(std::map<int, std::vector<int>> submissions) {
    SDL_GUI::Drawable *deadline_rect =
        this->_default_interface_model->find_first_drawable("deadline");
    const int arrow_height = Arrow::height(Arrow::Direction::UP);

    for (const auto &[timestamp, subs]: submissions) {
        const int distance_height = (subs.size() - 1) * interface_config.player.arrow_distance_px;
        int offset = deadline_rect->height() - arrow_height - distance_height;
        for (int job_id: subs) {
            const Job *job = this->_atlas_model->_jobs[job_id];
            /* TODO: get rid of magic number */
            SDL_GUI::Position position(10 + this->_interface_model->px_width(timestamp), offset);
            JobArrow *a = new JobArrow(job, timestamp, this->_interface_model, this->_atlas_model,
                                       position);
            this->_atlas_model->_drawables_jobs[a].insert(job->_id);
            deadline_rect->add_child(a);
            offset += interface_config.player.arrow_distance_px;
        }
    }
}


void AtlasController::create_deadline_drawables(std::map<int, std::vector<int>> deadlines) {
    SDL_GUI::Drawable *deadline_rect =
        this->_default_interface_model->find_first_drawable("deadline");

    for (const auto &[timestamp, dls]: deadlines) {
        const int distance_height = (dls.size() - 1) * interface_config.player.arrow_distance_px;
        int offset = distance_height;
        for (int job_id: dls) {
            const Job *job = this->_atlas_model->_jobs[job_id];
            /* TODO: get rid of magic number */
            SDL_GUI::Position position(10 + this->_interface_model->px_width(timestamp), offset);
            JobArrow *a = new JobArrow(job, timestamp, this->_interface_model, this->_atlas_model,
                                       position, Arrow::Direction::DOWN);
            this->_atlas_model->_drawables_jobs[a].insert(job->_id);
            deadline_rect->add_child(a);
            offset -= interface_config.player.arrow_distance_px;
        }
    }
}

void AtlasController::init_deadline_rect(std::map<int, std::vector<int>> submissions,
                                         std::map<int, std::vector<int>> deadlines) {
    std::map<int, int> heights;

    int sub_height = Arrow::height(Arrow::Direction::UP);

    /* submissions (map is empty yet) */
    for (auto const &[timestamp, subs]: submissions) {
        /* height of first arrow */
        heights[timestamp] = sub_height;

        /* height of other arrows (overlapping) */
        heights.at(timestamp) += (subs.size() - 1) * interface_config.player.arrow_distance_px;
    }

    int dl_height = Arrow::height(Arrow::Direction::DOWN);

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

void AtlasController::create_schedule_drawables() {
    std::vector<SDL_GUI::Drawable *> core_rects;
    SDL_GUI::Drawable *core_rect = this->_default_interface_model->find_first_drawable("core-0");
    core_rects.push_back(core_rect);
    SDL_GUI::Drawable *wrapper_rect = this->_default_interface_model->find_first_drawable("cores");
    for (int i = 1; i < this->_atlas_model->_n_cores; ++i) {
        SDL_GUI::Drawable *new_core = core_rect->deepcopy();
        new_core->move({0, i * (static_cast<int>(core_rect->height()) + interface_config.player.core_distance_px)});
        new_core->remove_attribute("core-0");
        std::stringstream ss;
        ss << "core-" << i;
        new_core->add_attribute(ss.str());
        core_rects.push_back(new_core);
        wrapper_rect->add_child(new_core);
    }

    for (Schedule *schedule: this->_atlas_model->_schedules) {
        /* constructing Schedule */
        ScheduleRect *r = new ScheduleRect(schedule, this->_interface_model, this->_player_model,
                                           this->_atlas_model);
        this->_atlas_model->_drawables_jobs[r].insert(schedule->_job->_id);
        core_rects[schedule->_core]->add_child(r);
    }
}

void AtlasController::init_cores_rect() {
    SDL_GUI::Drawable *deadline_rect = this->_default_interface_model->find_first_drawable("deadline");
    SDL_GUI::Drawable *cores_rect = this->_default_interface_model->find_first_drawable("cores");
    cores_rect->set_y(deadline_rect->position()._y + deadline_rect->height());

    SDL_GUI::Drawable *core_rect = this->_default_interface_model->find_first_drawable("core-0");
    core_rect->set_height(this->_interface_model->core_rect_height());
    std::vector<SDL_GUI::Drawable *> ds = core_rect->find("scheduler");
    for (SDL_GUI::Drawable *d: ds) {
        d->set_height(this->_interface_model->px_height(1));
    }

    SDL_GUI::Drawable *d = core_rect->find_first("atlas");
    d->set_y(this->_interface_model->scheduler_offset(SchedulerType::ATLAS));

    d = core_rect->find_first("recovery");
    d->set_y(this->_interface_model->scheduler_offset(SchedulerType::recovery));

    d = core_rect->find_first("cfs");
    d->set_y(this->_interface_model->scheduler_offset(SchedulerType::CFS));
}

void AtlasController::create_CFS_visibility_drawables(std::vector<EarlyCfsSchedule *> early_cfs_schedules) {
    for (EarlyCfsSchedule *schedule: early_cfs_schedules) {
        CfsVisibility visibility = schedule->create_visibility();
        std::stringstream rect_name;
        rect_name << "core-" << schedule->_core;
        SDL_GUI::Drawable *core_rect =
            this->_default_interface_model->find_first_drawable(rect_name.str());
        VisibilityLine *l = new VisibilityLine(this->_interface_model, this->_player_model,
                                               visibility, visibility._schedule);
        core_rect->add_child(l);
    }
}

void AtlasController::init_message_rect() {
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

void AtlasController::create_message_drawables(std::vector<Message *> messages) {
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
        this->_atlas_model->_drawables_jobs[t] = message->_jobs;
        this->_atlas_model->_drawables_messages[t] = message;
        message_rect->add_child(t);

        offset += t->height() + 2;

        SDL_GUI::Line *l = new SDL_GUI::Line(SDL_GUI::Position(5, offset - 3),
                                             SDL_GUI::Position(t->width(), offset - 3));
        l->hide();
        this->_atlas_model->_messages_underlines[message] = l;
        message_rect->add_child(l);
    }
    int overflow = offset - message_rect->height();
    std::cout << "overflow: " << overflow << std::endl;

    message_rect->set_min_limit({0, -std::max(0, overflow)});
}

void AtlasController::create_dependency_graph(std::vector<Job *> jobs) {
    std::map<unsigned, std::vector<Job *>> jobs_in_graph;
    for (Job *job: jobs) {
        jobs_in_graph[job->_dependency_level].push_back(job);
    }

    SDL_GUI::Drawable *dep_rect =
        this->_default_interface_model->find_first_drawable("dependencies");
    std::map<unsigned, JobRect *> rects;
    for (int i = 0; jobs_in_graph[i].size(); ++i) {
        int j = 0;
        for (Job *job: jobs_in_graph[i]) {
            JobRect *r = new JobRect(job, this->_interface_model, this->_atlas_model,
                                     {10 + 30 * j, 10 + 30 * i}, 20, 20);
            this->_atlas_model->_drawables_jobs[r].insert(job->_id);
            dep_rect->add_child(r);
            rects[job->_id] = r;
            ++j;
        }
    }

    for (Job *job: jobs) {
        JobRect *rect_from = rects[job->_id];
        for (Job *dep: job->known_dependencies()) {
            JobRect *rect_to = rects[dep->_id];
            SDL_GUI::Position begin = rect_from->position() + SDL_GUI::Position{10, 0};
            SDL_GUI::Position end = rect_to->position() + SDL_GUI::Position{10, 20};
            SDL_GUI::Line *l = new SDL_GUI::Line(begin, end);
            l->_style._color = SDL_GUI::RGB("black");
            dep_rect->add_child(l);
        }

        for (Job *dep: job->unknown_dependencies()) {
            JobRect *rect_to = rects[dep->_id];
            SDL_GUI::Position begin = rect_from->position() + SDL_GUI::Position{10, 0};
            SDL_GUI::Position end = rect_to->position() + SDL_GUI::Position{10, 20};
            SDL_GUI::Line *l = new SDL_GUI::Line(begin, end);
            l->_style._color = SDL_GUI::RGB("red");
            dep_rect->add_child(l);
        }
      }
}

void AtlasController::create_legend(std::vector<Job *> jobs) {
    SDL_GUI::Drawable *legend_rect = this->_default_interface_model->find_first_drawable("legend");

    SDL_GUI::Position position(15, 15);
    for (int i = 0; i < static_cast<int>(jobs.size()); ++i) {
        const Job *job = jobs[i];
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

SDL_GUI::Drawable *AtlasController::create_job_information(const Job *job) {
    /* create info text */
    std::stringstream ss;
    ss << "Job " << job->_id << std::endl;
    ss << "sub: " << job->_submission_time << std::endl;
    ss << "dl: " << job->_deadline << std::endl;
    ss << "est: " << job->_execution_time_estimate << std::endl;
    ss << "real: " << job->_execution_time << std::endl;
    SDL_GUI::Text *t = new SDL_GUI::Text(this->_default_interface_model->font(), ss.str(),
                                            {30, 5});

    /* add rect in appropriate color */
    JobRect *r = new JobRect(job, this->_interface_model, this->_atlas_model,
                                {5, 5}, 20, t->height());
    this->_atlas_model->_drawables_jobs[r].insert(job->_id);

    /* create Wrapper */
    SDL_GUI::Drawable *info = new SDL_GUI::Rect({0,0}, r->width() + t->width() + 10,
                                                t->height() + 10);
    info->add_child(t);
    info->add_child(r);

    return info;
}
