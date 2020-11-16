#include <controllers/atlas_controller.h>

#include <limits>

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/gui/primitives/rect.h>

#include <config/interface_config.h>
#include <gui/arrow.h>
#include <gui/message_text.h>
#include <gui/visibility_line.h>


static std::tuple<std::map<int, std::vector<int>>, int> submissions_from_jobs(std::vector<Job *> jobs);

static std::tuple<std::map<int, std::vector<int>>, int> deadlines_from_jobs(std::vector<Job *> jobs);

static void create_submission_drawables(std::map<int, std::vector<int>> submissions,
                                        SDL_GUI::Drawable *deadline_rect,
                                        std::function<int(float)> px_width,
                                        InterfaceModel *interface_model);

static void create_deadline_drawables(std::map<int, std::vector<int>> submissions,
                                      SDL_GUI::Drawable *deadline_rect,
                                      std::function<int(float)> px_width,
                                      InterfaceModel *interface_model);

static void create_schedule_drawables(InterfaceModel *interface_model,
                                      SDL_GUI::InterfaceModel *default_interface_model,
                                      AtlasModel *atlas_model,
                                      const PlayerModel *player_model,
                                      std::function<int(float)> px_width);

static void create_CFS_visibility_drawables(std::vector<CfsVisibility *> visibilities,
                                            InterfaceModel *interface_model,
                                            SDL_GUI::InterfaceModel *default_interface_model,
                                            const PlayerModel *player_model,
                                            AtlasModel *atlas_model);

static void create_message_drawables(std::vector<Message *> messages,
                                     SDL_GUI::InterfaceModel *default_interface_model,
                                     const PlayerModel *player_model);

static void hide_schedule_if_not_active(SDL_GUI::Drawable *d, std::function<int(float)> px_width,
                                        const PlayerModel *player_model, Schedule *schedule,
                                        std::map<SchedulerType, int> offsets);


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
    this->init_this();
}

void AtlasController::init_this() {
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
    create_submission_drawables(submissions, deadline_rect, px_width, this->_interface_model);
    create_deadline_drawables(deadlines, deadline_rect, px_width, this->_interface_model);


    create_schedule_drawables(this->_interface_model, this->_default_interface_model,
                              this->_atlas_model, this->_player_model, px_width);

    create_CFS_visibility_drawables(this->_atlas_model->_cfs_visibilities, this->_interface_model,
                                    this->_default_interface_model, this->_player_model,
                                    this->_atlas_model);
    create_message_drawables(this->_atlas_model->_messages, this->_default_interface_model,
                             this->_player_model);
}


void AtlasController::update() {
    if (this->_input_model->is_pressed(InputKey::QUIT)) {
        this->_application->_is_running = false;
    }
}

std::tuple<std::map<int, std::vector<int>>, int> submissions_from_jobs(std::vector<Job *> jobs) {
    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    for (const Job *job: jobs) {
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions =
            std::max(max_submissions, static_cast<int>(submissions[job->_submission_time].size()));
    }
    return std::make_tuple(submissions, max_submissions);
}


std::tuple<std::map<int, std::vector<int>>, int> deadlines_from_jobs(std::vector<Job *> jobs) {
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;
    for (const Job *job: jobs) {
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, static_cast<int>(deadlines[job->_deadline].size()));
    }
    return std::make_tuple(deadlines, max_deadlines);
}

void create_submission_drawables(std::map<int, std::vector<int>> submissions,
                                 SDL_GUI::Drawable *deadline_rect,
                                 std::function<int(float)> px_width,
                                 InterfaceModel *interface_model) {
    for (std::pair<int, std::vector<int>> submissions_at_time: submissions) {
        int submission_position_x = submissions_at_time.first;

        /* TODO: get rid of magic number */
        int offset = deadline_rect->height() - 7 * (submissions_at_time.second.size() - 1);
        for (int job_id: submissions_at_time.second) {
            Arrow *a = new Arrow(deadline_rect->position(), {10 + px_width(submission_position_x), offset});
            /* recompute the position based on the scale */
            a->add_recalculation_callback([submission_position_x,px_width](SDL_GUI::Drawable *d) {
                d->set_x(10 + px_width(submission_position_x));
            });
            a->_default_style._color = interface_model->get_color(job_id);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
}


void create_deadline_drawables(std::map<int, std::vector<int>> deadlines,
                               SDL_GUI::Drawable *deadline_rect,
                               std::function<int(float)> px_width,
                               InterfaceModel *interface_model) {
    for (std::pair<int, std::vector<int>> deadlines_at_time: deadlines) {
        int deadline_position_x = deadlines_at_time.first;

        /* TODO: get rid of magic number */
        int offset = 7 * (deadlines_at_time.second.size() - 1);
        for (int job_id: deadlines_at_time.second) {
            Arrow *a = new Arrow(deadline_rect->position(), {10 + px_width(deadline_position_x), offset}, Arrow::direction::DOWN);
            /* recompute the position based on the scale */
            a->add_recalculation_callback([deadline_position_x,px_width](SDL_GUI::Drawable *d) {
                d->set_x(10 + px_width(deadline_position_x));
            });
            a->_default_style._color = interface_model->get_color(job_id);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
    }
}


void create_schedule_drawables(InterfaceModel *interface_model,
                               SDL_GUI::InterfaceModel *default_interface_model,
                               AtlasModel *atlas_model,
                               const PlayerModel *player_model,
                               std::function<int(float)> px_width) {
    SDL_GUI::Drawable *core_rect = default_interface_model->find_first_drawable("core-1");

    std::map<SchedulerType, int> offsets;
    offsets[SchedulerType::ATLAS] = interface_config.schedule.ATLAS_offset_y;
    offsets[SchedulerType::recovery] = interface_config.schedule.recovery_offset_y;
    offsets[SchedulerType::CFS] = interface_config.schedule.CFS_offset_y;

    for (std::pair<int, Schedule *> s: atlas_model->_schedules) {
        Schedule *schedule = s.second;
        SDL_GUI::RGB color = interface_model->get_color(schedule->_job_id);

        /* constructing Schedule */
        SDL_GUI::Rect *r = new SDL_GUI::Rect();
        r->set_height(interface_model->px_height(1));
        r->_default_style._color = color;
        r->_default_style._has_background = true;
        r->_default_style._has_border = true;
        /* hide schedules that should not be shown */
        r->add_recalculation_callback([px_width, player_model, schedule, offsets] (SDL_GUI::Drawable *d) {
            hide_schedule_if_not_active(d, px_width, player_model, schedule, offsets);
        });
        /* add to tree */
        core_rect->add_child(r);
    }
}

void create_CFS_visibility_drawables(std::vector<CfsVisibility *> visibilities,
                                     InterfaceModel *interface_model,
                                     SDL_GUI::InterfaceModel *default_interface_model,
                                     const PlayerModel *player_model,
                                     AtlasModel *atlas_model) {
    SDL_GUI::Drawable *core_rect = default_interface_model->find_first_drawable("core-1");
    for (CfsVisibility *visibility: visibilities) {
        Schedule *schedule = atlas_model->_schedules.at(visibility->_schedule_id);
        VisibilityLine *l = new VisibilityLine(interface_model, player_model, visibility, schedule);
        core_rect->add_child(l);
    }
}


void hide_schedule_if_not_active(SDL_GUI::Drawable *d, std::function<int(float)> px_width,
                                 const PlayerModel *player_model, Schedule *schedule,
                                 std::map<SchedulerType, int> offsets) {
    if (not schedule->exists_at_time(player_model->_position)) {
        d->hide();
        return;
    }
    d->show();
    int begin;
    SchedulerType scheduler;
    int execution_time;
    std::tie(begin, scheduler, execution_time) = schedule->get_data_at_time(player_model->_position);

    d->set_x(px_width(begin));
    d->set_y(offsets.at(scheduler));
    d->set_width(px_width(execution_time));
}

void create_message_drawables(std::vector<Message *> messages,
                              SDL_GUI::InterfaceModel *default_interface_model,
                              const PlayerModel *player_model) {
    SDL_GUI::Drawable *message_rect = default_interface_model->find_first_drawable("messages");
    int offset = 0;
    for (Message *message: messages) {
        MessageText *t = new MessageText(message->_message, default_interface_model->font(), message_rect->width(), offset);
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
