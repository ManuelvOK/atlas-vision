#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

#include <SDL2/SDL2_gfxPrimitives.h>

#include <model.h>
#include <viewmodel.h>
#include <controller.h>
#include <schedule_rect.h>
#include <vision_config.h>


/**
 * calculate render positions for all jobs.
 *
 * This is to check mouse hovering between positioning and rendering
 */
static void calculate_render_positions();


/**
 * prepare rendering of jobs in Schedule view
 */
static void render_jobs_in_schedule();

/**
 * prepare rendering of one schedule
 *
 * @param schedule_id
 *   id of schedule to render
 * @param rect
 *   Position to render
 */
static void render_schedule(Schedule_rect &schedule);

/**
 * prepare rendering of jobs in EDF view
 */
static void render_jobs_in_EDF_view();

/**
 * prepare rendering of job deadlines in EDF view
 */
static void render_deadlines();

/* TODO: documentation */
static void render_submissions();
static void render_visibilities();

/**
 * prepare rendering for everything that concerns the simulation playing
 */
static void render_player();

/**
 * prepare rendering for current position marker in player
 */
static void render_player_position();
void calculate_job_in_EDF_view(const Job &job, int offset) {
    viewmodel.EDF_schedules.emplace_back(&config, job.id, offset, 0, job.execution_time_estimate, 1);
    auto &deadlines = viewmodel.deadlines;

    if (deadlines.find(job.deadline) == deadlines.end()) {
        deadlines.insert({job.deadline, std::vector<int>()});
    }
    deadlines[job.deadline].push_back(job.id);

    auto &submissions = viewmodel.submissions;

    if (submissions.find(job.submission_time) == submissions.end()) {
        submissions.insert({job.submission_time, std::vector<int>()});
    }
    submissions[job.submission_time].push_back(job.id);
}


void calculate_render_positions() {

    for (auto p: model->schedules) {
        const Schedule &s = p.second;
        calculate_schedule_render_position(s);
    }

    /* precompute positions for deadlines */
    int i = 0;
    /* iterate through all deadlines. They are ordered in vectors that are mapped to a specific
     * timestamp */
    for (std::pair<int, std::vector<int>> p: viewmodel.deadlines) {
        /* x position is the timestamp the deadlines describe */
        int deadline_position_x = u_to_px_w(p.first) + config.window.margin_x_px;
        /* create rect for position rendering. Width and Height are not neccessary since the
         * deadlines are rendered as arrows */
        SDL_Rect *r = &viewmodel.deadlines_render_positions[i].r;
        r->x = deadline_position_x + u_to_px_w(config.deadline.margin_x_u);
        r->y = config.window.margin_y_px + u_to_px_w(config.deadline.margin_y_u);
        //r->w = u_to_px_w(config.deadline.width_u);
        //r->h = u_to_px_h(config.deadline.height_u);

        /* TODO: get rid of magic number */
        int offset = 7 * (p.second.size() - 1);
        for (int id: p.second) {
            r = &viewmodel.deadline_history_render_positions[id].r;
            r->x = deadline_position_x;
            r->y = config.window.margin_y_px
                + u_to_px_h(config.deadline.margin_y_u + config.deadline.height_u
                            + config.color_deadline.spacing_u)
                + offset;
            //r->w = u_to_px_w(config.color_deadline.width_u);
            //r->h = u_to_px_h(config.color_deadline.height_u);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
        ++i;
    }
}

void render_jobs_in_schedule() {
    for (Schedule_rect &s: viewmodel.schedules) {
        render_schedule(s);
    }
}

void render_visibilities() {
    float timestamp = model->player.position;
    for (auto v: model->cfs_visibilities) {
        if (v.is_active_at_time(timestamp)) {
            Schedule_rect &s = viewmodel.schedules[v.schedule_id];

            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderDrawLine(renderer,
                               config.window.margin_x_px + u_to_px_w(s.x),
                               config.window.margin_y_px +
                               u_to_px_h(config.schedule.ATLAS_offset_y_u + config.schedule.offset_y_u + 0.5),
                               config.window.margin_x_px + u_to_px_w(timestamp),
                               /* TODO: get rid of magic 0.5 */
                               config.window.margin_y_px +
                               u_to_px_h(config.schedule.CFS_offset_y_u + config.schedule.offset_y_u + 0.5)
                               );
        }
    }
}

void render_player() {
    /* render lines for schedulers */
    render_horizontal_player_line(config.schedule.ATLAS_offset_y_u + config.schedule.offset_y_u,
                                  config.schedule.ATLAS_grey);
    render_horizontal_player_line(config.schedule.recovery_offset_y_u + config.schedule.offset_y_u,
                                  config.schedule.recovery_grey);
    render_horizontal_player_line(config.schedule.CFS_offset_y_u + config.schedule.offset_y_u,
                                  config.schedule.CFS_grey);

    /* render unit markings over the whole whindow */
    for (int i = 0; i <= model->player.max_position; ++i) {
        int x = config.window.margin_x_px + u_to_px_w(i);
        int color = (i % 5) ? 155 : 100;
        SDL_SetRenderDrawColor(renderer, color, color, color, 255);
        SDL_RenderDrawLine(renderer, x, config.window.margin_y_px, x,
                           config.window.height_px - config.window.margin_y_px);
    }


#if 0
    /* render Points of interest */
    /* begin whith getting maximum value for submission count */
    int max_submissions = 0;
    for (Player_state s: model->player.states) {
        max_submissions = std::max(max_submissions, s.n_submissions);
    }

    /* actually render new submissions into player overwiev */
    for (Player_state s: model->player.states) {
        SDL_Rect r;
        /* Why this + 1 and -1 ? please explain */
        r.x = config.window.margin_x_px + u_to_px_w(s.begin) + 1;
        r.y = u_to_px_h(config.player.offset_y_u + 1) - 1;
        r.w = u_to_px_w(config.player.poi_width_u);
        r.h = -u_to_px_h(config.player.poi_max_height_u) / max_submissions * s.n_submissions;
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
        SDL_RenderFillRect(renderer, &r);
    }
#endif
}

void render_player_position() {
    /* render current position */
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    float x = config.window.margin_x_px + u_to_px_w(model->player.position);
    SDL_RenderDrawLine(renderer, x, config.window.margin_y_px / 2, x,
                       config.window.height_px - config.window.margin_y_px / 2);
}
void DeadlineFrame::update_submission_positions() {
    for (std::pair<int, std::vector<int>> p: this->viewmodel.submissions) {
        int submission_position_x = u_to_px_w(p.first) + this->viewmodel.config.window.margin_x_px;

        /* TODO: get rid of magic number */
        int offset = -7 * (p.second.size() - 1);
        for (int job: p.second) {
            SDL_Rect *r = &this->viewmodel.submission_render_positions[job].r;
            r->x = submission_position_x;
            r->y = u_to_px_h(this->viewmodel.config.schedule.offset_y_u)
                   + this->viewmodel.config.window.margin_y_px - 1 + offset;
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
}

void SchedulerFrame::update_schedule_render_position(const Model *model, const Schedule &schedule) {
    float timestamp = model->player.position;
    /* precompute positions for job in schedule view */
    Schedule_rect *s = &this->viewmodel.schedules[schedule.id];

    int begin;
    float execution_time;
    scheduler_type scheduler;
    std::tie(begin, scheduler, execution_time) = schedule.get_data_at_time(timestamp);

    s->visible = schedule.exists_at_time(timestamp);
    s->x = begin;
    s->w = execution_time;
    s->h = 1;

    switch (scheduler) {
        case scheduler_type::ATLAS:
            s->y = this->viewmodel.config.schedule.offset_y_u
                + this->viewmodel.config.schedule.ATLAS_offset_y_u;
            break;
        case scheduler_type::recovery:
            s->y = this->viewmodel.config.schedule.offset_y_u
                + this->viewmodel.config.schedule.recovery_offset_y_u;
            break;
        case scheduler_type::CFS:
            s->y = this->viewmodel.config.schedule.offset_y_u
                + this->viewmodel.config.schedule.CFS_offset_y_u;
            if (schedule.is_active_at_time(timestamp)) {
                s->w = timestamp - begin;
            }
            break;
    }
}

void View::calculate() {
    /* TODO: this should be done recursively over the frame tree */
    this->window_frame.precompute();
#if 0
    this->viewmodel.n_jobs = this->model->jobs.size();
    this->viewmodel.n_schedules = this->model->schedules.size();

    /* the order of the job list can't be changed because the jobs are handled by id, which is the
     * position inside the list. That means we have to sort another list */
    std::vector<int> EDF_sorted_jobs;
    EDF_sorted_jobs.reserve(this->model->jobs.size());
    for (unsigned i = 0; i < this->model->jobs.size(); ++i) {
        EDF_sorted_jobs.push_back(i);
    }
    std::sort(EDF_sorted_jobs.begin(), EDF_sorted_jobs.end(), [](int a, int b) {
            return this->model->jobs.at(a).deadline < this->model->jobs.at(b).deadline;
        });

    /* calculate jobs in EDF view */
    /* TODO: is this needed any longer? */
#if 0
    int max_deadline = 0;
    int offset = 0;
    for (int i: EDF_sorted_jobs) {
        const Job &job = this->model->jobs.at(i);
        calculate_job_in_EDF_view(job, offset);
        offset += job.execution_time_estimate;
        max_deadline = std::max(max_deadline, job.deadline);
        viewmodel.deadline_history_render_positions.push_back({{0,0,0,0},0,true});
        viewmodel.submission_render_positions.push_back({{0, 0, 0, 0}, 0, true});
    }
#endif

    for (auto p: model->schedules) {
        const Schedule &s = p.second;
        /* create SDL rects for render positions */
        viewmodel.schedules.emplace_back(&config, s.job_id);
    }
    /* create SDL rects for deadline render positions */
    for (unsigned i = 0; i < viewmodel.deadlines.size(); ++i) {
        viewmodel.deadlines_render_positions.push_back({{0,0,0,0},0,0});
    }
    config.player.width_u = std::max(offset,max_deadline);
    recompute_config();
#endif
}
