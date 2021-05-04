#include <cbs/cbs_view_controller.h>

#include <gui/budget_line.h>
#include <gui/job_rect.h>
#include <gui/schedule_rect.h>

CbsViewController::CbsViewController(SDL_GUI::ApplicationBase *application,
                                     CbsSimulationModel *cbs_model,
                                     InterfaceModel *interface_model,
                                     SDL_GUI::InterfaceModel *default_interface_model,
                                     InputModel *input_model,
                                     const PlayerModel *player_model)
    : SimulationViewController(application, cbs_model, interface_model, default_interface_model,
                               input_model, player_model),
      _cbs_model(cbs_model) {}

void CbsViewController::init() {
    SimulationViewController::init();
    this->create_budget_lines(this->_cbs_model->_servers);
}

void CbsViewController::create_schedule_drawables() {
    SDL_GUI::Drawable *core_rect = this->_default_interface_model->find_first_drawable("core-0");

    for (BaseSchedule *schedule: this->_cbs_model->schedules()) {
        ScheduleRect *r = new ScheduleRect(schedule, this->_interface_model,
                                           this->_player_model, this->_cbs_model);
        this->_cbs_model->_drawables_jobs[r].insert(schedule->job()->_id);
        core_rect->add_child(r);
    }

}

void CbsViewController::init_cores_rect() {
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
    d->set_y(this->_interface_model->scheduler_offset(0));

    d = core_rect->find_first("recovery");
    d->set_y(this->_interface_model->scheduler_offset(1));

    d = core_rect->find_first("cfs");
    core_rect->remove_children([d](SDL_GUI::Drawable *drawable){return drawable == d;});
}

SDL_GUI::Drawable *CbsViewController::create_job_information(const CbsJob *job) {
    /* create info text */
    std::stringstream ss;
    ss << "Job " << job->_id << std::endl;
    ss << "sub: " << job->_submission_time << std::endl;
    ss << "time: " << job->_execution_time << std::endl;
    SDL_GUI::Text *t1 = new SDL_GUI::Text(this->_default_interface_model->font(), ss.str(),
                                          {30, 5});

    ss.str("");
    ss << "dl: " << job->deadline(0) << std::endl;
    SDL_GUI::Text *t2 = new SDL_GUI::Text(this->_default_interface_model->font(), ss.str(),
                                          {30, static_cast<int>(5 + t1->height())});

    std::vector<SoftRtJob *> soft_rt_jobs = this->_cbs_model->_soft_rt_jobs;
    if (std::find(soft_rt_jobs.begin(), soft_rt_jobs.end(), job) != soft_rt_jobs.end()) {
        /* recalculate deadline */
        t2->add_recalculation_callback(
            [this, t2, job](SDL_GUI::Drawable *d){
                (void) d;
                unsigned timestamp = this->_player_model->_position;
                std::map<const CbsJob *, unsigned> deadlines;
                unsigned deadline = job->deadline(timestamp);
                if (deadlines[job] == deadline) {
                    return;
                }
                deadlines[job] = deadline;
                std::stringstream ss;
                ss << "dl: " << deadline;
                t2->set_text(ss.str());
            });
    }


    /* add rect in appropriate color */
    JobRect *r = new JobRect(job, this->_interface_model, this->_cbs_model,
                             {5, 5}, 20, t1->height() + t2->height());
    this->_cbs_model->_drawables_jobs[r].insert(job->_id);

    /* create Wrapper */
    SDL_GUI::Drawable *info =
        new SDL_GUI::Rect({0,0}, r->width() + std::max(t1->width(), t2->width()) + 10,
                          t1->height() + t2->height() + 10);
    info->add_child(t1);
    info->add_child(t2);
    info->add_child(r);

    return info;
}

std::vector<JobArrow *>
CbsViewController::create_deadline_drawables(std::map<unsigned, std::vector<unsigned>> deadlines) {
    std::vector<JobArrow *> arrows =
        SimulationViewController<CbsSchedule, CbsJob>::create_deadline_drawables(deadlines);

    std::map<const BaseJob *, SoftRtJob *> job_mapping;
    for (SoftRtJob *job: this->_cbs_model->_soft_rt_jobs) {
        job_mapping.emplace(job, job);
    }

    /* add recalculation callback for position */
    for (JobArrow *arrow: arrows) {
        const BaseJob *base_job = arrow->job();
        unsigned deadline = base_job->deadline(0);
        if (not this->_cbs_model->n_dls.contains(deadline)) {
            this->_cbs_model->n_dls[deadline] = 0;
        }
        this->_cbs_model->n_dls[deadline]++;
        if (not job_mapping.contains(base_job)) {
            continue;
        }
        arrow->hide();
        SoftRtJob *job = job_mapping.at(base_job);
        arrow->add_recalculation_callback(
            [this, job, arrow](SDL_GUI::Drawable *d){
                static std::map<SoftRtJob *, unsigned> deadlines;

                unsigned timestamp = this->_player_model->_position;
                unsigned deadline = job->deadline(timestamp);
                if (deadlines[job] == deadline) {
                    return;
                }

                this->_cbs_model->n_dls[deadlines[job]]--;
                if (not this->_cbs_model->n_dls.contains(deadline)) {
                    this->_cbs_model->n_dls[deadline] = 0;
                }
                unsigned offset = this->_cbs_model->n_dls[deadline]++;

                deadlines[job] = deadline;
                if (deadline == 0) {
                    d->hide();
                } else {
                    d->show();
                }

                arrow->set_pos_x(deadline);
                arrow->set_y(interface_config.player.arrow_distance_px * offset);

                d->parent()->sort_children(
                    [](SDL_GUI::Drawable *a, SDL_GUI::Drawable *b){
                        return a->position()._y > b->position()._y;
                    });
            });
    }
    return arrows;
}

void
CbsViewController::create_budget_lines(const std::map<unsigned, ConstantBandwidthServer> &servers) {
    for (const auto [_, cbs]: servers) {
        this->create_budget_line(cbs);
    }
}

void CbsViewController::create_budget_line(const ConstantBandwidthServer &cbs) {
    std::vector<SoftRtSchedule *> schedules(cbs._schedules.begin(), cbs._schedules.end());

    std::sort(schedules.begin(), schedules.end(),
        [](SoftRtSchedule *a, SoftRtSchedule *b) {
            return a->last_data()._begin < b->last_data()._begin;
        });

    SDL_GUI::Drawable *core_rect = this->_default_interface_model->find_first_drawable("core-0");

    int y_offset = 10 + (interface_config.unit.height_px + interface_config.player.scheduler_distance_px) * 2;
    int height = interface_config.unit.height_px;

    unsigned budget_before = cbs.max_budget();
    unsigned time_before = 0;
    int value_before = y_offset;

    for (SoftRtSchedule *schedule: schedules) {
        CbsScheduleData data = schedule->last_data();

        /* add point at beginning of schedule */
        unsigned current_time = data._begin;

        /* add horizontal line */
        if (current_time != time_before) {
            BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, time_before, current_time, value_before, value_before);
            core_rect->add_child(l);
        }

        /* check for filling at begin of schedule */
        if (cbs.budget_fill_times().contains(current_time) && budget_before != cbs.max_budget()) {
            BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, current_time, current_time, y_offset, value_before);
            core_rect->add_child(l);

            value_before = y_offset;
            budget_before = cbs.max_budget();

        }

        time_before = current_time;
        unsigned execution_time_left = data._execution_time;

        /* add points for filling up */
        while (execution_time_left >= budget_before) {
            current_time += budget_before;

            /* add Line for budget decrease */
            BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, time_before, current_time, value_before, y_offset + height);
            core_rect->add_child(l);

            /* add line for budget fill */
            l = new BudgetLine(this->_interface_model, this->_player_model, current_time, current_time, y_offset, y_offset + height);
            core_rect->add_child(l);

            value_before = y_offset;

            execution_time_left -= budget_before;
            budget_before = cbs.max_budget();
            time_before = current_time;
        }

        if (execution_time_left == 0) {
            continue;
        }

        current_time += execution_time_left;
        /* add line for final budget decrease */
        float factor = 1 - execution_time_left * 1.0 / cbs.max_budget();
        unsigned current_value = value_before + height * factor;
        BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, time_before, current_time, value_before, current_value);
        core_rect->add_child(l);

        value_before = current_value;
        budget_before -= execution_time_left;
        time_before = current_time;
    }

    BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, time_before, time_before + 2000, value_before, value_before);
    core_rect->add_child(l);
}

//void CbsViewController::create_budget_line(const ConstantBandwidthServer &cbs) {
//    std::map<unsigned, unsigned> budget_line = cbs.budget_line();
//
//    SDL_GUI::Drawable *core_rect = this->_default_interface_model->find_first_drawable("core-0");
//
//    /* for now we draw after the first CBS. TODO: change the 2 to something reasonable */
//    int y_offset = 10 + (interface_config.unit.height_px + interface_config.player.scheduler_distance_px) * 2;
//
//    int height = interface_config.unit.height_px;
//    unsigned value_before = y_offset;
//    unsigned budget_before = cbs.max_budget();
//    unsigned time_before = 0;
//    for (const auto [timestamp, budget]: budget_line) {
//        if (time_before == timestamp) {
//            continue;
//        }
//
//        if (budget_before == budget) {
//            BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, time_before, timestamp, value_before, value_before);
//            core_rect->add_child(l);
//
//            time_before = timestamp;
//            continue;
//        }
//
//        float factor = 1 - static_cast<float>(budget_before - budget) / cbs.max_budget();
//        unsigned value = value_before + height * factor;
//        BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, time_before, timestamp, value_before, value);
//        core_rect->add_child(l);
//
//        value_before = value;
//        budget_before = budget;
//        time_before = timestamp;
//
//        /* do vertical line if 0 (fill) */
//        if (budget == 0) {
//            BudgetLine *l = new BudgetLine(this->_interface_model, this->_player_model, timestamp, timestamp, y_offset, y_offset + height);
//            core_rect->add_child(l);
//
//            budget_before = cbs.max_budget();
//            value_before = y_offset;
//            continue;
//        }
//
//        /* do line from last point to new point */
//    }
//}

