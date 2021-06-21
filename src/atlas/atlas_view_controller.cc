#include <atlas/atlas_view_controller.h>

#include <algorithm>
#include <limits>
#include <sstream>

#include <SDL_GUI/gui/drawable.h>
#include <SDL_GUI/gui/primitives/text.h>
#include <SDL_GUI/gui/rgb.h>

#include <config/interface_config.h>
#include <gui/job_rect.h>
#include <gui/schedule_rect.h>
#include <gui/visibility_line.h>

AtlasViewController::AtlasViewController(SDL_GUI::ApplicationBase *application,
                                         AtlasSimulationModel *atlas_model,
                                         InterfaceModel *interface_model,
                                         SDL_GUI::InterfaceModel *default_interface_model,
                                         InputModel *input_model,
                                         const PlayerModel *player_model)
    : SimulationViewController(application, atlas_model, interface_model, default_interface_model,
                             input_model, player_model),
      _atlas_model(atlas_model) {}

void AtlasViewController::init() {
    SimulationViewController::init();
    this->create_CFS_visibility_drawables(this->_atlas_model->_early_cfs_schedules);
    this->create_dependency_graph(this->_atlas_model->_jobs);
}

void AtlasViewController::reset() {
    SimulationViewController::reset();
    this->_default_interface_model->find_first_drawable("dependencies")->remove_all_children();
}

void AtlasViewController::create_schedule_drawables() {
    std::vector<SDL_GUI::Drawable *> core_rects;
    SDL_GUI::Drawable *core_rect = this->_default_interface_model->find_first_drawable("core-0");
    core_rects.push_back(core_rect);
    SDL_GUI::Drawable *wrapper_rect = this->_default_interface_model->find_first_drawable("cores");
    for (int i = 1; i < static_cast<int>(this->_atlas_model->_n_cores); ++i) {
        SDL_GUI::Drawable *new_core = core_rect->deepcopy();
        new_core->move({0, i * (static_cast<int>(core_rect->height()) + interface_config.player.core_distance_px)});
        new_core->remove_attribute("core-0");
        std::stringstream ss;
        ss << "core-" << i;
        new_core->add_attribute(ss.str());
        core_rects.push_back(new_core);
        wrapper_rect->add_child(new_core);
    }

    for (BaseAtlasSchedule *schedule: this->_atlas_model->_schedules) {
        /* constructing Schedule */
        ScheduleRect *r = new ScheduleRect(schedule, this->_interface_model, this->_player_model,
                                           this->_atlas_model);
        this->_atlas_model->_drawables_jobs[r].insert(schedule->job()->_id);
        core_rects[schedule->_core]->add_child(r);
    }
}

void AtlasViewController::init_cores_rect() {
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
    d->set_y(this->_interface_model->scheduler_offset(2));
}

void AtlasViewController::create_CFS_visibility_drawables(std::vector<EarlyCfsSchedule *> early_cfs_schedules) {
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


void AtlasViewController::create_dependency_graph(std::vector<AtlasJob *> jobs) {
    std::map<unsigned, std::vector<AtlasJob *>> jobs_in_graph;
    for (AtlasJob *job: jobs) {
        jobs_in_graph[job->_dependency_level].push_back(job);
    }

    SDL_GUI::Drawable *dep_rect =
        this->_default_interface_model->find_first_drawable("dependencies");
    std::map<unsigned, JobRect *> rects;
    for (unsigned i = 0; jobs_in_graph[i].size(); ++i) {
        int j = 0;
        for (AtlasJob *job: jobs_in_graph[i]) {
            JobRect *r = new JobRect(job, this->_interface_model, this->_atlas_model,
                                     {10 + 30 * j, 10 + 30 * static_cast<int>(i)}, 20, 20);
            this->_atlas_model->_drawables_jobs[r].insert(job->_id);
            dep_rect->add_child(r);
            rects[job->_id] = r;
            ++j;
        }
    }

    for (AtlasJob *job: jobs) {
        JobRect *rect_from = rects[job->_id];
        for (AtlasJob *dep: job->known_dependencies()) {
            JobRect *rect_to = rects[dep->_id];
            SDL_GUI::Position begin = rect_from->position() + SDL_GUI::Position{10, 0};
            SDL_GUI::Position end = rect_to->position() + SDL_GUI::Position{10, 20};
            SDL_GUI::Line *l = new SDL_GUI::Line(begin, end);
            l->_style._color = SDL_GUI::RGB("black");
            dep_rect->add_child(l);
        }

        for (AtlasJob *dep: job->unknown_dependencies()) {
            JobRect *rect_to = rects[dep->_id];
            SDL_GUI::Position begin = rect_from->position() + SDL_GUI::Position{10, 0};
            SDL_GUI::Position end = rect_to->position() + SDL_GUI::Position{10, 20};
            SDL_GUI::Line *l = new SDL_GUI::Line(begin, end);
            l->_style._color = SDL_GUI::RGB("red");
            dep_rect->add_child(l);
        }
      }
}

SDL_GUI::Drawable *AtlasViewController::create_job_information(const AtlasJob *job) {
    /* create info text */
    std::stringstream ss;
    ss << "Job " << job->_id << std::endl;
    ss << "sub: " << job->_submission_time << std::endl;
    ss << "dl: " << job->_deadline << std::endl;
    ss << "est: " << job->_execution_time_estimate << std::endl;
    SDL_GUI::Text *t1 = new SDL_GUI::Text(this->_default_interface_model->font(), ss.str(),
                                          {30, 5});

    ss.str("");
    ss << "real: " << job->_execution_time << std::endl;
    SDL_GUI::Text *t2 = new SDL_GUI::Text(this->_default_interface_model->font(), ss.str(),
                                          {30, static_cast<int>(5 + t1->height())});

    if (job->_execution_time_estimate < job->_execution_time) {
        t2->set_color(SDL_GUI::RGB(155, 0, 0, 255));
    } else if (job->_execution_time_estimate > job->_execution_time) {
        t2->set_color(SDL_GUI::RGB(0, 155, 0, 255));
    }

    /* add rect in appropriate color */
    JobRect *r = new JobRect(job, this->_interface_model, this->_atlas_model,
                             {5, 5}, 20, t1->height() + t2->height());
    this->_atlas_model->_drawables_jobs[r].insert(job->_id);

    /* create Wrapper */
    SDL_GUI::Drawable *info =
        new SDL_GUI::Rect({0,0}, r->width() + std::max(t1->width(), t2->width()) + 10,
                          t1->height() + t2->height() + 10);
    info->add_child(t1);
    info->add_child(t2);
    info->add_child(r);

    return info;
}
