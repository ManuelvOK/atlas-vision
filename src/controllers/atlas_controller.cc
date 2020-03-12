#include <controllers/atlas_controller.h>

#include <limits>

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/gui/primitives/rect.h>
#include <SDL_GUI/inc/util/tree.h>

#include <config/interface_config.h>
#include <gui/arrow.h>

#include <iostream>

AtlasController::AtlasController(AtlasModel *atlas_model, InterfaceModel *interface_model, const SDL_GUI::InputModel<InputKey> *input_model, const PlayerModel *player_model) : _atlas_model(atlas_model), _interface_model(interface_model), _input_model(input_model), _player_model(player_model) {
    this->init_this();
}

void AtlasController::init_this() {
    this->_interface_model->init_colors(this->_atlas_model->_jobs.size());
    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;
    for (const Job *job: this->_atlas_model->_jobs) {
        /* subs */
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions = std::max(max_submissions, static_cast<int>(submissions[job->_submission_time].size()));

        /* dls */
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, static_cast<int>(deadlines[job->_deadline].size()));
    };

    SDL_GUI::TreeNode<SDL_GUI::Drawable> *deadline_rect = this->_interface_model->find_first_tree_node("deadline");

    std::function<int(float)> px_width = std::bind(&InterfaceModel::px_width, this->_interface_model, std::placeholders::_1);
    /* create subs */
    for (std::pair<int, std::vector<int>> submissions_at_time: submissions) {
        int submission_position_x = submissions_at_time.first;

        /* TODO: get rid of magic number */
        int offset = deadline_rect->node()->height() - 7 * (submissions_at_time.second.size() - 1);
        for (int job_id: submissions_at_time.second) {
            Arrow *a = new Arrow({0, offset - 1});
            /* recompute the position based on the scale */
            a->add_recalculation_callback([submission_position_x,px_width](SDL_GUI::Drawable *d) {
                    d->set_x(10 + px_width(submission_position_x));
                });
            a->_default_style._color = this->_interface_model->get_color(job_id);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }

    /* create deadlines */
    for (std::pair<int, std::vector<int>> deadlines_at_time: deadlines) {
        int deadline_position_x = deadlines_at_time.first;

        /* TODO: get rid of magic number */
        int offset = 7 * (deadlines_at_time.second.size() - 1);
        for (int job_id: deadlines_at_time.second) {
            Arrow *a = new Arrow({deadline_position_x, offset-1}, Arrow::direction::DOWN);
            /* recompute the position based on the scale */
            a->add_recalculation_callback([deadline_position_x,px_width](SDL_GUI::Drawable *d) {
                    d->set_x(10 + px_width(deadline_position_x));
                });
            a->_default_style._color = this->_interface_model->get_color(job_id);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
    }

    std::map<SchedulerType, int> offsets;
    offsets[SchedulerType::ATLAS] = interface_config.schedule.ATLAS_offset_y;
    offsets[SchedulerType::recovery] = interface_config.schedule.recovery_offset_y;
    offsets[SchedulerType::CFS] = interface_config.schedule.CFS_offset_y;
    SDL_GUI::TreeNode<SDL_GUI::Drawable> *core_rect = this->_interface_model->find_first_tree_node("core-1");

    /* create schedules */
    for (std::pair<int, Schedule *> s: this->_atlas_model->_schedules) {
        Schedule *schedule = s.second;
        SDL_GUI::RGB color = this->_interface_model->get_color(schedule->_job_id);

            /* constructing Schedule */
            SDL_GUI::Rect *r = new SDL_GUI::Rect();
            r->set_height(this->_interface_model->px_height(1));
            r->_default_style._color = color;
            r->_default_style._has_background = true;
            r->_default_style._has_border = true;
            const PlayerModel *player_model = this->_player_model;
            /* hide schedules that should not be shown */
            r->add_recalculation_callback([px_width, player_model, schedule, offsets](SDL_GUI::Drawable *d){
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
                });
            /* add to tree */
            core_rect->add_child(r);
    }
}

void AtlasController::update() {
}
