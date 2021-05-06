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
    SDL_GUI::Drawable *sidebar = this->_default_interface_model->find_first_drawable("sidebar");
    sidebar->hide();
    this->_default_interface_model->find_first_drawable("messages")->set_x(sidebar->position()._x);
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
    d->remove_attribute("atlas");
    d->add_attribute("hard-rt");
    d->set_y(this->_interface_model->scheduler_offset(0));

    d = core_rect->find_first("recovery");
    d->remove_attribute("recovery");
    d->add_attribute("soft-rt");
    d->add_attribute("soft-rt-0");
    unsigned offset = this->_interface_model->scheduler_offset(1);
    d->set_y(offset);
    d->set_height(35);

    for (unsigned i = 1; i < this->_cbs_model->_servers.size(); ++i) {
        d = d->deepcopy();
        d->remove_attribute("soft-rt-0");
        std::stringstream ss;
        ss << "soft-rt-" << i;
        d->add_attribute(ss.str());
        core_rect->add_child(d);
        d->set_height(35);
        offset += 40;
        d->set_y(offset);
    }

    core_rect->set_height(offset + 40);


    d = core_rect->find_first("cfs");
    core_rect->remove_children([d](SDL_GUI::Drawable *drawable){return drawable == d;});
}

void CbsViewController::create_legend(std::vector<CbsJob *> jobs) {
    (void) jobs;
    SDL_GUI::Drawable *legend_rect = this->_default_interface_model->find_first_drawable("legend");

    SDL_GUI::Rect *hard_rt_job_rect = new SDL_GUI::Rect({5, 5}, legend_rect->width() - 10, 0);
    hard_rt_job_rect->_style._has_border = true;
    hard_rt_job_rect->_style._border_color = SDL_GUI::RGB("black");
    SDL_GUI::Position position(10, 10);
    SDL_GUI::Drawable *last_added = nullptr;
    for (const HardRtJob *job: this->_cbs_model->_hard_rt_jobs) {
        SDL_GUI::Drawable *info = this->create_job_information(job);
        info->set_position(position);
        hard_rt_job_rect->add_child(info);
        last_added = info;

        position._x += info->width() + 10;

        if (position._x + info->width() > hard_rt_job_rect->width() - 20) {
            position._x = 10;
            position._y += info->height() + 10;
        }
    }

    if (last_added) {
        unsigned height = last_added->position()._y + last_added->height() + 10;
        hard_rt_job_rect->set_height(height);
    }

    legend_rect->add_child(hard_rt_job_rect);

    std::map<const ConstantBandwidthServer *, std::vector<const SoftRtJob *>> cbs_jobs;

    for (const SoftRtJob *job: this->_cbs_model->_soft_rt_jobs) {
        cbs_jobs[job->_cbs].push_back(job);
    }

    int offset = hard_rt_job_rect->height() + 10;
    for (const auto [cbs, jobs]: cbs_jobs) {
        SDL_GUI::Rect *cbs_rect = new SDL_GUI::Rect({5, offset}, legend_rect->width() - 10, 0);
        cbs_rect->_style._has_border = true;
        cbs_rect->_style._border_color = SDL_GUI::RGB("black");
        position = SDL_GUI::Position(10, 10);
        SDL_GUI::Drawable *last_added = nullptr;
        for (const SoftRtJob *job: jobs) {
            SDL_GUI::Drawable *info = this->create_job_information(job);
            info->set_position(position);
            cbs_rect->add_child(info);
            last_added = info;

            position._x += info->width() + 10;

            if (position._x + info->width() > cbs_rect->width() - 20) {
                position._x = 10;
                position._y += info->height() + 10;
            }
        }

        if (last_added) {
            unsigned height = last_added->position()._y + last_added->height() + 10;
            cbs_rect->set_height(height);
        }

        offset += cbs_rect->height() + 5;
        legend_rect->add_child(cbs_rect);
    }
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
CbsViewController::create_submission_drawables(std::map<unsigned, std::vector<unsigned>> deadlines) {
    std::vector<JobArrow *> arrows =
        SimulationViewController<CbsSchedule, CbsJob>::create_submission_drawables(deadlines);
    for (JobArrow *arrow: arrows) {
        this->_cbs_model->_arrows.emplace(arrow, arrow);
    }
    return arrows;
}

std::vector<JobArrow *>
CbsViewController::create_deadline_drawables(std::map<unsigned, std::vector<unsigned>> deadlines) {
    this->_cbs_model->_dl_arrows =
        SimulationViewController<CbsSchedule, CbsJob>::create_deadline_drawables(deadlines);

    for (SoftRtJob *job: this->_cbs_model->_soft_rt_jobs) {
        this->_cbs_model->_soft_rt_job_mapping.emplace(job, job);
    }

    for (JobArrow *arrow: this->_cbs_model->_dl_arrows) {
        this->_cbs_model->_arrows.emplace(arrow, arrow);
        if (this->_cbs_model->_soft_rt_job_mapping.contains(arrow->job())) {
            arrow->hide();
        }
    }

    return this->_cbs_model->_dl_arrows;
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

    int y_offset = this->_interface_model->scheduler_offset(cbs.id() * 2 + 2) - 2;
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
        float factor = execution_time_left * 1.0 / cbs.max_budget();
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

void CbsViewController::update() {
    SimulationViewController::update();
    static std::map<SoftRtJob *, unsigned> deadlines;
    bool deadline_changed = false;

    /* add recalculation callback for position */
    for (JobArrow *arrow: this->_cbs_model->_dl_arrows) {
        const BaseJob *base_job = arrow->job();

        if (not this->_cbs_model->_soft_rt_job_mapping.contains(base_job)) {
            continue;
        }

        SoftRtJob *job = this->_cbs_model->_soft_rt_job_mapping.at(base_job);
        unsigned timestamp = this->_player_model->_position;
        unsigned deadline = job->deadline(timestamp);
        if (deadlines[job] == deadline) {
            continue;
        }

        /* hide if deadline does not exist yet */
        deadlines[job] = deadline;
        if (deadline == 0) {
            arrow->hide();
        } else {
            arrow->show();
        }

        arrow->set_pos_x(deadline);
        deadline_changed = true;
    }

    if (not deadline_changed) {
        return;
    }

    /* set y position */
    std::map<unsigned, unsigned> n_deadlines;
    for (JobArrow *arrow: this->_cbs_model->_dl_arrows) {
        unsigned deadline = arrow->job()->deadline(this->_player_model->_position);
        if (not n_deadlines.contains(deadline)) {
            n_deadlines[deadline] = 0;
        }

        unsigned offset = n_deadlines[deadline] * interface_config.player.arrow_distance_px;
        arrow->set_y(offset);

        n_deadlines[deadline]++;
    }

    /* sort deadline drawables */
    this->_cbs_model->_dl_arrows.front()->parent()->sort_children(
        [this](SDL_GUI::Drawable *a, SDL_GUI::Drawable *b) {
            /* non arrows are "smaller" */
            if (not this->_cbs_model->_arrows.contains(a)) {
                return true;
            }
            if (not this->_cbs_model->_arrows.contains(b)) {
                return false;
            }

            JobArrow *arrow_a = this->_cbs_model->_arrows[a];
            JobArrow *arrow_b = this->_cbs_model->_arrows[b];

            Arrow::Direction direction_a = arrow_a->direction();
            Arrow::Direction direction_b = arrow_b->direction();

            /* deadlines are "smaller" than submissions */
            if (direction_a != direction_b) {
                if (direction_a == Arrow::Direction::DOWN) {
                    return true;
                } else {
                    return false;
                }
            }

            /* deadlines stack up */
            if (direction_a == Arrow::Direction::DOWN) {
                return arrow_a->position()._y > arrow_b->position()._y;
            }

            /* submissions stack down */
            return arrow_a->position()._y < arrow_b->position()._y;
        });
}

