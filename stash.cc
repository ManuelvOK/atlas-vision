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

struct job_rect {
    SDL_Rect r;
    int job_id;
    bool visible;
};

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
