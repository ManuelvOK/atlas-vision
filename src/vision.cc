#include <vision.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

#include <SDL2/SDL2_gfxPrimitives.h>

#include <model.h>
#include <controller.h>
#include <schedule_rect.h>
#include <vision_config.h>

/**
 * SDL Window
 */
static SDL_Window *window;

/**
 * SDL Renderer
 */
static SDL_Renderer *renderer;

/**
 * application model
 */
static const Model *model = nullptr;

Vision_config config;

struct job_rect {
    SDL_Rect r;
    int job_id;
    bool visible;
};

static int n_jobs;
static int n_schedules;
static std::vector<Schedule_rect> schedules;
static std::vector<Schedule_rect> EDF_schedules;
static std::vector<struct job_rect> deadlines_render_positions;
static std::vector<struct job_rect> deadline_history_render_positions;
static std::vector<struct job_rect> submission_render_positions;
static std::map<int, std::vector<int>> deadlines;
static std::map<int, std::vector<int>> submissions;
static std::vector<unsigned> colors;

/**
 * set render color to preset colors have to be initialised first
 *
 * @param job
 *   job number
 * @param alpha
 *   color alpha
 */
static void set_color(int job, float modifier = 1.0);

/* TODO: Documentation */
static void get_color(int job, float modifier, int *r, int *g, int *b);

/**
 * initialise color presets
 *
 * @param n_jobs
 *   number of jobs to calculate color preset for
 */
static void init_colors(int n_jobs);

/**
 * convert HSV color values to RGB ones
 *
 * @param h
 *   h value for HSV color code
 * @param s
 *   s value for HSV color code
 * @param v
 *   v value for HSV color code
 * @param[out] r
 *   r value for RGB color code
 * @param[out] g
 *   g value for RGB color code
 * @param[out] b
 *   b value for RGB color code
 */
static void HSV_to_RGB(float h, float s, float v,
                       float *r, float *g, float *b);

/**
 * calculate render positions for all jobs.
 *
 * This is to check mouse hovering between positioning and rendering
 */
static void calculate_render_positions();
/* TODO: documentation */
static void calculate_submission_positions();

/**
 * check if point is inside a SDL_Rect
 *
 * @param x
 *   x coordinate of point to check
 * @param y
 *   y coordinate of point to check
 *
 * @return
 *   true if Point is inside SDL_Rect. Otherwise False
 */
static bool point_inside_rect(int x, int y, const SDL_Rect *r);

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

static int u_to_px_w(float unit);
static int u_to_px_h(float unit);
//static float px_to_u_w(int pixel);
//static float px_to_u_h(int pixel);

static void recompute_config();

void exit_SDL_DestroyWindow(int status, void *window) {
    (void) status;
    SDL_DestroyWindow(static_cast<SDL_Window *>(window));
}

void exit_SDL_DestroyRenderer(int status, void *renderer) {
    (void) status;
    SDL_DestroyRenderer(static_cast<SDL_Renderer *>(renderer));
}

void init_graphics(const Model *_model) {
    model = _model;
    window = SDL_CreateWindow("visualisation", 0, 0, 640, 480, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        std::cerr << "unable to create window: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyWindow, window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        std::cerr << "unable to create renderer: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyRenderer, renderer);
}

void calculate_job_in_EDF_view(const Job &job, int offset) {
    EDF_schedules.emplace_back(&config, job.id, offset, 0, job.execution_time_estimate, 1);

    if (deadlines.find(job.deadline) == deadlines.end()) {
        deadlines.insert({job.deadline, std::vector<int>()});
    }
    deadlines[job.deadline].push_back(job.id);

    if (submissions.find(job.submission_time) == submissions.end()) {
        submissions.insert({job.submission_time, std::vector<int>()});
    }
    submissions[job.submission_time].push_back(job.id);
}

void calculate_vision() {
    n_jobs = model->jobs.size();
    n_schedules = model->schedules.size();

    /* the order of the job list can't be changed because the jobs are handled by id, which is the
     * position inside the list. That means we have to sort another list */
    std::vector<int> EDF_sorted_jobs;
    EDF_sorted_jobs.reserve(model->jobs.size());
    for (unsigned i = 0; i < model->jobs.size(); ++i) {
        EDF_sorted_jobs.push_back(i);
    }
    std::sort(EDF_sorted_jobs.begin(), EDF_sorted_jobs.end(), [](int a, int b) {
            return model->jobs.at(a).deadline < model->jobs.at(b).deadline;
        });

    /* calculate jobs in EDF view */
    int max_deadline = 0;
    int offset = 0;
    for (int i: EDF_sorted_jobs) {
        const Job &job = model->jobs.at(i);
        calculate_job_in_EDF_view(job, offset);
        offset += job.execution_time_estimate;
        max_deadline = std::max(max_deadline, job.deadline);
        deadline_history_render_positions.push_back({{0,0,0,0},0,true});
        submission_render_positions.push_back({{0, 0, 0, 0}, 0, true});
    }

    for (auto p: model->schedules) {
        const Schedule &s = p.second;
        /* create SDL rects for render positions */
        schedules.emplace_back(&config, s.job_id);
    }
    /* create SDL rects for deadline render positions */
    for (unsigned i = 0; i < deadlines.size(); ++i) {
        deadlines_render_positions.push_back({{0,0,0,0},0,0});
    }
    config.player.width_u = std::max(offset,max_deadline);
    recompute_config();
}

void calculate_schedule_render_position(const Schedule &schedule) {
    float timestamp = model->player.position;
    /* precompute positions for job in schedule view */
    Schedule_rect *s = &schedules[schedule.id];

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
            s->y = config.schedule.offset_y_u + config.schedule.ATLAS_offset_y_u;
            break;
        case scheduler_type::recovery:
            s->y = config.schedule.offset_y_u + config.schedule.recovery_offset_y_u;
            break;
        case scheduler_type::CFS:
            s->y = config.schedule.offset_y_u + config.schedule.CFS_offset_y_u;
            if (schedule.is_active_at_time(timestamp)) {
                s->w = timestamp - begin;
            }
            break;
    }
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
    for (std::pair<int, std::vector<int>> p: deadlines) {
        /* x position is the timestamp the deadlines describe */
        int deadline_position_x = u_to_px_w(p.first) + config.window.margin_x_px;
        /* create rect for position rendering. Width and Height are not neccessary since the
         * deadlines are rendered as arrows */
        SDL_Rect *r = &deadlines_render_positions[i].r;
        r->x = deadline_position_x + u_to_px_w(config.deadline.margin_x_u);
        r->y = config.window.margin_y_px + u_to_px_w(config.deadline.margin_y_u);
        //r->w = u_to_px_w(config.deadline.width_u);
        //r->h = u_to_px_h(config.deadline.height_u);

        /* TODO: get rid of magic number */
        int offset = 7 * (p.second.size() - 1);
        for (int id: p.second) {
            r = &deadline_history_render_positions[id].r;
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

void calculate_submission_positions() {
    for (std::pair<int, std::vector<int>> p: submissions) {
        int submission_position_x = u_to_px_w(p.first) + config.window.margin_x_px;

        /* TODO: get rid of magic number */
        int offset = -7 * (p.second.size() - 1);
        for (int job: p.second) {
            SDL_Rect *r = &submission_render_positions[job].r;
            r->x = submission_position_x;
            r->y = u_to_px_h(config.schedule.offset_y_u) + config.window.margin_y_px - 1 + offset;
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
}

void render_vision() {
    calculate_render_positions();
    calculate_submission_positions();

    /* init colors if not happend before */
    if (colors.empty()) {
        init_colors(model->jobs.size());
    }

    /* paint background white */
    SDL_SetRenderDrawColor(renderer, 170, 170, 170, 0);
    SDL_RenderClear(renderer);

    /* render player */
    render_player();

    /* render job views */
    render_jobs_in_schedule();
    render_jobs_in_EDF_view();
    render_deadlines();
    render_submissions();

    render_visibilities();

    /* render current position on top of all */
    render_player_position();

    SDL_RenderPresent(renderer);

    /* reset config_changed flag */
    config.changed = false;
}

void render_schedule(Schedule_rect &schedule) {
    if (!schedule.visible) {
        return;
    }
    float modifier = (schedule.job_id == model->hovered_job) ? 1.0 : 0.9;
    set_color(schedule.job_id, modifier);

    SDL_RenderFillRect(renderer, schedule.render_position());
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    SDL_RenderDrawRect(renderer, schedule.render_position());
}

void render_jobs_in_schedule() {
    for (Schedule_rect &s: schedules) {
        render_schedule(s);
    }
}

void render_jobs_in_EDF_view() {
    for (Schedule_rect &s: EDF_schedules) {
        float modifier = (s.job_id == model->hovered_job) ? 1.0 : 0.9;
        set_color(s.job_id, modifier * 0.8);
        SDL_RenderFillRect(renderer, s.render_position());
    }
}

void render_submissions() {
    short x[9] = {-10, 10, 10, 40, 50, 0, -50, -40, -10};
    short y[9] = {0, 0, -80, -50, -60, -110, -60, -50, -80};

    /* draw deadline in EDF view */
    for (auto p: submissions) {
        for (auto i: p.second) {
            short pos_x[9];
            short pos_y[9];
            SDL_Rect r = submission_render_positions[i].r;
            for (int j = 0; j < 9; ++j) {
                /* TODO: get rid of magic 5 */
                pos_x[j] = x[j] / 5.0 + r.x;
                pos_y[j] = y[j] / 5.0 + r.y;
            }
            int c_r, c_g, c_b;
            get_color(i, 1, &c_r, &c_g, &c_b);
            filledPolygonRGBA(renderer, pos_x, pos_y, 9, c_r, c_g, c_b, 255);
            //polygonRGBA(renderer, pos_x, pos_y, 9, 0, 0, 0, 255);
            //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            //SDL_RenderFillRect(renderer, &r);
        }
    }
}

void render_visibilities() {
    float timestamp = model->player.position;
    for (auto v: model->cfs_visibilities) {
        if (v.is_active_at_time(timestamp)) {
            Schedule_rect &s = schedules[v.schedule_id];

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

void render_deadlines() {
    /* draw job history for each deadline */
    /*
    for (int i = 0; i < n_jobs; ++i) {
        float modifier = (i == model->hovered_job) ? 1.0 : 0.9;
        SDL_Rect r = deadline_history_render_positions[i].r;
        set_color(i, modifier);
        SDL_RenderFillRect(renderer, &r);
    }
    */
    short x[9] = {-10, 10, 10, 40, 50, 0, -50, -40, -10};
    short y[9] = {0, 0, 80, 50, 60, 110, 60, 50, 80};

    /* draw deadline in EDF view */
    for (unsigned i = 0; i < n_jobs; ++i) {
        short pos_x[9];
        short pos_y[9];
        SDL_Rect r = deadline_history_render_positions[i].r;
        for (int j = 0; j < 9; ++j) {
            /* TODO: get rid of magic 5 */
            pos_x[j] = x[j] / 5.0 + r.x;
            pos_y[j] = y[j] / 5.0 + r.y;
        }
        int c_r, c_g, c_b;
        get_color(i, 1, &c_r, &c_g, &c_b);
        filledPolygonRGBA(renderer, pos_x, pos_y, 9, c_r, c_g, c_b, 255);
        //polygonRGBA(renderer, pos_x, pos_y, 9, 0, 0, 0, 255);
        //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        //SDL_RenderFillRect(renderer, &r);
    }
}

void render_horizontal_player_line(float offset_y_u, int grey) {
    SDL_Rect r;
    r.x = config.window.margin_x_px;
    r.y = u_to_px_h(offset_y_u) + config.window.margin_y_px;
    r.w = u_to_px_w(model->player.max_position);
    r.h = config.unit.height_px;

    SDL_SetRenderDrawColor(renderer, grey, grey, grey, 255);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, r.x, r.y, r.x + r.w, r.y);
    SDL_RenderDrawLine(renderer, r.x, r.y + r.h, r.x + r.w, r.y + r.h);
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

void init_colors(int n_jobs) {
    std::vector<unsigned> c;
    for (int i = 0; i < n_jobs; ++i) {
        c.push_back((360 / (n_jobs)) * i);
    }
    int half = c.size() / 2;
    colors.reserve(n_jobs);
    for (int i = 0; i < half; ++i) {
        colors.push_back(c[half + i]);
        colors.push_back(c[i]);
    }
    if (c.size() % 2 == 1) {
        colors.push_back(c.back());
    }
}

void set_color(int job, float modifier) {
    float r = 0;
    float g = 0;
    float b = 0;

    HSV_to_RGB((float)colors[job], 0.7f, 0.9f * modifier, &r, &g, &b);
    SDL_SetRenderDrawColor(renderer, r * 255, g * 255, b * 255, 255);
}

void get_color(int job, float modifier, int *r, int *g, int *b) {
    float red = 0;
    float green = 0;
    float blue = 0;

    HSV_to_RGB((float)colors[job], 0.7f, 0.9f * modifier, &red, &green, &blue);
    *r = red * 255;
    *g = green * 255;
    *b = blue * 255;
}

void HSV_to_RGB(float h, float s, float v, float *r, float *g, float *b) {

    int i;
    float f, p, q, t;
    if (s == 0) {
        /* achromatic (grey) */
        *r = *g = *b = v;
        return;
    }

    /* sector 0 to 5 */
    h /= 60;
    i = floor(h);

    /* factorial part of h */
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));
    switch(i) {
        case 0: *r = v; *g = t; *b = p; break;
        case 1: *r = q; *g = v; *b = p; break;
        case 2: *r = p; *g = v; *b = t; break;
        case 3: *r = p; *g = q; *b = v; break;
        case 4: *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

int get_hovered_job(int x, int y) {
    for (Schedule_rect &s: EDF_schedules) {
        if (point_inside_rect(x, y, s.render_position())) {
            return s.job_id;
        }
    }
    for (Schedule_rect &s: schedules) {
        if (point_inside_rect(x, y, s.render_position())) {
            return s.job_id;
        }
    }
    return -1;
}

bool point_inside_rect(int x, int y, const SDL_Rect *r) {
    return x > r->x && x < r->x + r->w
        && y > r->y && y < r->y + r->h;
}

float position_in_player(int x, int y) {
    (void) y;
    x -= config.window.margin_x_px;
    if (x < 0) {
        return 0;
    }
    if (x > u_to_px_w(config.player.width_u)) {
        return model->player.max_position;
    }
    return x * 1.0f / u_to_px_w(config.player.width_u) * model->player.max_position;
}

void recompute_config() {
    config.player.width_u = model->player.max_position;
    config.unit.width_px =
        (config.window.width_px - 2 * config.window.margin_x_px) / config.player.width_u;
    config.changed = true;
}

void update_window(int width, int height) {
    config.window.width_px = width;
    config.window.height_px = height;
    recompute_config();
}

int u_to_px_w(float unit) {
    return (unit * config.unit.width_px);
}

int u_to_px_h(float unit) {
    return (unit * config.unit.height_px);
}

float px_to_u_w(int pixel) {
    return (pixel / config.unit.width_px);
}

float px_to_u_h(int pixel) {
    return (pixel / config.unit.height_px);
}
