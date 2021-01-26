#include <controllers/atlas_controller.h>

#include <algorithm>
#include <limits>
#include <sstream>

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>

#include <config/interface_config.h>
#include <gui/job_arrow.h>
#include <gui/job_rect.h>
#include <gui/message_text.h>
#include <gui/schedule_rect.h>
#include <gui/visibility_line.h>


/**
 * Get all the submissions there are from a list of jobs grouped by submission time
 * @param jobs list of jobs to get submissions for
 * @return job submissions grouped by submission time
 */
static std::tuple<std::map<int, std::vector<int>>, int>
submissions_from_jobs(std::vector<Job *> jobs);

/**
 * Get all the deadlines there are from a list of jobs grouped by deadline time
 * @param jobs list of jobs to get deadlines for
 * @return job deadlines grouped by deadline time
 */
static std::tuple<std::map<int, std::vector<int>>, int>
deadlines_from_jobs(std::vector<Job *> jobs);

/**
 * Create Drawables for all the submissions (Submission Arrows that is)
 * @param submissions job submissions ordered by submission time
 * @param deadline_rect parent drawable to add submissions as childs to
 * @param px_width function to calculate the current width in px from a unit
 * @param interface_model the applications interface model
 * @param atlas_model the simulations model
 */
static void create_submission_drawables(std::map<int, std::vector<int>> submissions,
                                        SDL_GUI::Drawable *deadline_rect,
                                        std::function<int(float)> px_width,
                                        InterfaceModel *interface_model,
                                        AtlasModel *atlas_model);

/**
 * Create Drawables for all the deadlines (Deadline Arrows that is)
 * @param deadlines job submissions ordered by deadlines time
 * @param deadline_rect parent drawable to add deadlines as childs to
 * @param px_width function to calculate the current width in px from a unit
 * @param interface_model the applications interface model
 * @param atlas_model the simulations model
 */
static void create_deadline_drawables(std::map<int, std::vector<int>> deadlines,
                                      SDL_GUI::Drawable *deadline_rect,
                                      std::function<int(float)> px_width,
                                      InterfaceModel *interface_model,
                                      AtlasModel *atlas_model);

/**
 * Create drawables for all schedules and register calbacks to reposition/hide them on occasion.
 * @param interface_model the applicatoins interface model
 * @param default_interface_model the interface model of the SDL_GUI library
 * @param atlas_model the applications atlas model
 * @param player_model the applications player model
 */
static void create_schedule_drawables(InterfaceModel *interface_model,
                                      SDL_GUI::InterfaceModel *default_interface_model,
                                      AtlasModel *atlas_model,
                                      const PlayerModel *player_model);

/**
 * Create drawables for all visibilities
 * @param visibilities list of visibilities to add drawables for
 * @param interface_model the applicatoins interface model
 * @param default_interface_model the interface model of the SDL_GUI library
 * @param atlas_model the applications atlas model
 * @param player_model the applications player model
 */
static void create_CFS_visibility_drawables(std::vector<CfsVisibility *> visibilities,
                                            InterfaceModel *interface_model,
                                            SDL_GUI::InterfaceModel *default_interface_model,
                                            const PlayerModel *player_model);


/**
 * create drawables for all the event messages
 * @param messages list of messages to create drawables for
 * @param default_interface_model the interface model of the SDL_GUI library
 * @param player_model the applications player model
 */
static void create_message_drawables(std::vector<Message *> messages,
                                     SDL_GUI::InterfaceModel *default_interface_model,
                                     const PlayerModel *player_model);

/**
 * create all the drawables for the dependency graph of the jobs
 * @param jobs jobs to draw dependency graph for
 * @param default_interface_model the interface model of the SDL_GUI library
 * @param interface_model the applicatoins interface model
 */
static void create_dependency_graph(std::vector<Job *> jobs,
                                    SDL_GUI::InterfaceModel *default_interface_model,
                                    InterfaceModel *interface_model,
                                    AtlasModel *atlas_model);

/**
 * create all the drawables for the visual color to job number mapping
 * @param jobs jobs to create legend for
 * @param default_interface_model the interface model of the SDL_GUI library
 * @param interface_model the applicatoins interface model
 */
static void create_legend(std::vector<Job *> jobs, SDL_GUI::InterfaceModel *default_interface_model,
                          InterfaceModel *interface_model,
                          AtlasModel *atlas_model);

AtlasController::AtlasController(SDL_GUI::ApplicationBase *application, AtlasModel *atlas_model,
                                 InterfaceModel *interface_model,
                                 SDL_GUI::InterfaceModel *default_interface_model,
                                 const SDL_GUI::InputModel<InputKey> *input_model,
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

    SDL_GUI::Drawable *deadline_rect =
        this->_default_interface_model->find_first_drawable("deadline");

    std::function<int(float)> px_width = std::bind(&InterfaceModel::px_width,
                                                   this->_interface_model, std::placeholders::_1);
    create_submission_drawables(submissions, deadline_rect, px_width, this->_interface_model,
                                this->_atlas_model);
    create_deadline_drawables(deadlines, deadline_rect, px_width, this->_interface_model,
                              this->_atlas_model);


    create_schedule_drawables(this->_interface_model, this->_default_interface_model,
                              this->_atlas_model, this->_player_model);

    create_CFS_visibility_drawables(this->_atlas_model->_cfs_visibilities, this->_interface_model,
                                    this->_default_interface_model, this->_player_model);
    create_message_drawables(this->_atlas_model->_messages, this->_default_interface_model,
                             this->_player_model);
    create_dependency_graph(this->_atlas_model->_jobs, this->_default_interface_model,
                            this->_interface_model, this->_atlas_model);
    create_legend(this->_atlas_model->_jobs, this->_default_interface_model,
                  this->_interface_model, this->_atlas_model);

    this->_atlas_model->_dirty = false;
}

void AtlasController::reinit() {
    std::cout << "reinitialising atlas_controller" << std::endl;
    this->_default_interface_model->find_first_drawable("core-1")->remove_children(
        [](SDL_GUI::Drawable *d){
            return d->_type != "Rect";
        });
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
    std::map<const SDL_GUI::Drawable *, const Job *> drawables_jobs = this->_atlas_model->_drawables_jobs;
    auto first_hovered = std::find_if(hovered.begin(), hovered.end(),
        [drawables_jobs, mouse_position](SDL_GUI::Drawable *d){
            return drawables_jobs.contains(d) && not d->is_hidden()
                   && d->is_inside_clip_rect(mouse_position);
        });
    if (first_hovered != hovered.end()) {
        const Job *job = this->_atlas_model->_drawables_jobs[*first_hovered];
        this->_atlas_model->_highlighted_job = job->_id;
    } else {
        this->_atlas_model->_highlighted_job = -1;
    }
}

static std::tuple<std::map<int, std::vector<int>>, int>
submissions_from_jobs(std::vector<Job *> jobs) {
    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    for (const Job *job: jobs) {
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions =
            std::max(max_submissions, static_cast<int>(submissions[job->_submission_time].size()));
    }
    return std::make_tuple(submissions, max_submissions);
}


static std::tuple<std::map<int, std::vector<int>>, int>
deadlines_from_jobs(std::vector<Job *> jobs) {
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;
    for (const Job *job: jobs) {
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, static_cast<int>(deadlines[job->_deadline].size()));
    }
    return std::make_tuple(deadlines, max_deadlines);
}

static void create_submission_drawables(std::map<int, std::vector<int>> submissions,
                                        SDL_GUI::Drawable *deadline_rect,
                                        std::function<int(float)> px_width,
                                        InterfaceModel *interface_model, AtlasModel *atlas_model) {
    for (std::pair<int, std::vector<int>> submissions_at_time: submissions) {
        int submission_position_x = submissions_at_time.first;

        /* TODO: get rid of magic number */
        int offset = deadline_rect->height() - 22 - 7 * (submissions_at_time.second.size() - 1);
        for (int job_id: submissions_at_time.second) {
            const Job *job = atlas_model->_jobs[job_id];
            SDL_GUI::Position position(10 + px_width(submission_position_x), offset);
            JobArrow *a = new JobArrow(job, submission_position_x, interface_model, atlas_model,
                                       position);
            atlas_model->_drawables_jobs[a] = job;
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
}


static void create_deadline_drawables(std::map<int, std::vector<int>> deadlines,
                                      SDL_GUI::Drawable *deadline_rect,
                                      std::function<int(float)> px_width,
                                      InterfaceModel *interface_model, AtlasModel *atlas_model) {
    for (std::pair<int, std::vector<int>> deadlines_at_time: deadlines) {
        int deadline_position_x = deadlines_at_time.first;

        /* TODO: get rid of magic number */
        int offset = 7 * (deadlines_at_time.second.size() - 1);
        for (int job_id: deadlines_at_time.second) {
            const Job *job = atlas_model->_jobs[job_id];
            SDL_GUI::Position position(10 + px_width(deadline_position_x), offset);
            JobArrow *a = new JobArrow(job, deadline_position_x, interface_model, atlas_model,
                                       position, Arrow::Direction::DOWN);
            atlas_model->_drawables_jobs[a] = job;
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
    }
}


static void create_schedule_drawables(InterfaceModel *interface_model,
                                      SDL_GUI::InterfaceModel *default_interface_model,
                                      AtlasModel *atlas_model,
                                      const PlayerModel *player_model) {
    SDL_GUI::Drawable *core_rect = default_interface_model->find_first_drawable("core-1");

    std::map<SchedulerType, int> offsets;
    offsets[SchedulerType::ATLAS] = interface_config.schedule.ATLAS_offset_y;
    offsets[SchedulerType::recovery] = interface_config.schedule.recovery_offset_y;
    offsets[SchedulerType::CFS] = interface_config.schedule.CFS_offset_y;

    for (Schedule * schedule: atlas_model->_schedules) {
        //std::cout << "creating Schedule drawable for schedule " << schedule->_id << std::endl;
        /* constructing Schedule */
        ScheduleRect *r = new ScheduleRect(schedule, interface_model, player_model, atlas_model,
                                           offsets);
        atlas_model->_drawables_jobs[r] = atlas_model->_jobs[schedule->_job->_id];
        core_rect->add_child(r);
    }
}

static void create_CFS_visibility_drawables(std::vector<CfsVisibility *> visibilities,
                                            InterfaceModel *interface_model,
                                            SDL_GUI::InterfaceModel *default_interface_model,
                                            const PlayerModel *player_model) {
    SDL_GUI::Drawable *core_rect = default_interface_model->find_first_drawable("core-1");
    for (CfsVisibility *visibility: visibilities) {
        Schedule *schedule = visibility->_schedule;
        VisibilityLine *l = new VisibilityLine(interface_model, player_model, visibility, schedule);
        core_rect->add_child(l);
    }
}

static void create_message_drawables(std::vector<Message *> messages,
                                     SDL_GUI::InterfaceModel *default_interface_model,
                                     const PlayerModel *player_model) {
    SDL_GUI::Drawable *message_rect = default_interface_model->find_first_drawable("messages");
    int offset = 0;
    for (Message *message: messages) {
        std::stringstream ss;
        ss << message->_timestamp << ": " << message->_message;
        MessageText *t = new MessageText(ss.str(), default_interface_model->font(),
                                         message_rect->width(), offset);
        t->add_recalculation_callback([player_model, message, t](SDL_GUI::Drawable *) {
            if (player_model->_position >= message->_timestamp) {
                t->activate();
            } else {
                t->deactivate();
            }
        });
        message_rect->add_child(t);
        offset += t->height();
    }
}

static void create_dependency_graph(std::vector<Job *> jobs,
                                    SDL_GUI::InterfaceModel *default_interface_model,
                                    InterfaceModel *interface_model, AtlasModel *atlas_model) {
    std::map<unsigned, std::vector<Job *>> jobs_in_graph;
    for (Job *job: jobs) {
        jobs_in_graph[job->_dependency_level].push_back(job);
    }

    SDL_GUI::Drawable *dep_rect = default_interface_model->find_first_drawable("dependencies");
    std::map<unsigned, JobRect *> rects;
    for (int i = 0; jobs_in_graph[i].size(); ++i) {
        int j = 0;
        for (Job *job: jobs_in_graph[i]) {
            JobRect *r = new JobRect(job, interface_model, atlas_model,
                                     {10 + 30 * j, 10 + 30 * i}, 20, 20);
            atlas_model->_drawables_jobs[r] = job;
            dep_rect->add_child(r);
            rects[job->_id] = r;
            ++j;
        }
    }

    for (Job *job: jobs) {
        JobRect *rect_from = rects[job->_id];
        for (Job *dep: job->_known_dependencies) {
            JobRect *rect_to = rects[dep->_id];
            SDL_GUI::Position begin = rect_from->position() + SDL_GUI::Position{10, 0};
            SDL_GUI::Position end = rect_to->position() + SDL_GUI::Position{10, 20};
            SDL_GUI::Line *l = new SDL_GUI::Line(begin, end - begin);
            l->_default_style._color = SDL_GUI::RGB("black");
            dep_rect->add_child(l);
        }

        for (Job *dep: job->_unknown_dependencies) {
            JobRect *rect_to = rects[dep->_id];
            SDL_GUI::Position begin = rect_from->position() + SDL_GUI::Position{10, 0};
            SDL_GUI::Position end = rect_to->position() + SDL_GUI::Position{10, 20};
            SDL_GUI::Line *l = new SDL_GUI::Line(begin, end - begin);
            l->_default_style._color = SDL_GUI::RGB("red");
            dep_rect->add_child(l);
        }
      }
}

static void create_legend(std::vector<Job *> jobs, SDL_GUI::InterfaceModel *default_interface_model,
                          InterfaceModel *interface_model, AtlasModel *atlas_model) {
    SDL_GUI::Drawable *legend_rect = default_interface_model->find_first_drawable("legend");

    unsigned width = 0;
    for (int i = 0; i < static_cast<int>(jobs.size()); ++i) {
        std::stringstream ss;
        ss << i << ":";
        SDL_GUI::Text *t = new SDL_GUI::Text(default_interface_model->font(), ss.str(),
                                             {5, 10 + 25 * i});
        legend_rect->add_child(t);
        width = std::max(width, t->width());
    }
    for (int i = 0; i < static_cast<int>(jobs.size()); ++i) {
        JobRect *r = new JobRect(jobs[i], interface_model, atlas_model,
                                 {10 + static_cast<int>(width), 7 + 25 * i}, 20, 20);
        atlas_model->_drawables_jobs[r] = jobs[i];
        legend_rect->add_child(r);
    }
}
