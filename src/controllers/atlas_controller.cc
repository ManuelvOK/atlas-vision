#include <controllers/atlas_controller.h>

#include <limits>

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/gui/primitives/rect.h>
#include <SDL_GUI/inc/util/tree.h>

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

    std::map<SchedulerType, SDL_GUI::TreeNode<SDL_GUI::Drawable> *> schedulers;
    schedulers[SchedulerType::ATLAS] = this->_interface_model->find_first_tree_node("atlas");
    schedulers[SchedulerType::recovery] = this->_interface_model->find_first_tree_node("recovery");
    schedulers[SchedulerType::CFS] = this->_interface_model->find_first_tree_node("cfs");
    /* create schedules */
    for (std::pair<int, Schedule *> s: this->_atlas_model->_schedules) {
        Schedule *schedule = s.second;
        SDL_GUI::RGB color = this->_interface_model->get_color(schedule->_job_id);
        /* get a list of all different time interval starts */
        std::vector<int> beginning_times;
        for (std::pair<int,SchedulerType> pair: schedule->_scheduler) {
            beginning_times.push_back(pair.first);
        }
        /* add INT_MAX as end of the last interval */
        beginning_times.push_back(std::numeric_limits<int>::max());
        /* iterate over all time intervals. Since we added INT_MAX as last value, we do not want to iterate over that */
        for (unsigned i = 0; i < beginning_times.size() - 1; ++i) {
            int starting_time = beginning_times[i];
            int end_time = beginning_times[i+1];
            if (schedule->_end >= 0) {
                end_time = std::min(schedule->_end, end_time);
            }
            SchedulerType scheduler = schedule->_scheduler[starting_time];
            /* constructing Schedule */
            SDL_GUI::Rect *r = new SDL_GUI::Rect();
            r->set_height(this->_interface_model->px_height(1));
            r->_default_style._color = color;
            r->_default_style._has_background = true;
            r->_default_style._has_border = true;
            /* zooming callback */
            r->add_recalculation_callback([px_width,starting_time,schedule](SDL_GUI::Drawable *d){
                    d->set_x(px_width(schedule->_begin[starting_time]));
                    d->set_width(px_width(schedule->_execution_time[starting_time]));
                });
            const PlayerModel *player_model = this->_player_model;
            /* hide schedules that should not be shown */
            r->add_recalculation_callback([player_model,starting_time, end_time](SDL_GUI::Drawable *d){
                    if (player_model->_position >= starting_time && player_model->_position < end_time) {
                        d->show();
                    } else {
                        d->hide();
                    }
                });
            /* add to tree */
            schedulers[scheduler]->add_child(r);
        }
    }
}

void AtlasController::update() {
}
