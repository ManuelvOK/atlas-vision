#include <vision.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

#include <model.h>
#include <controller.h>

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

static struct {
    struct {
        int width_px = 1000;
        int height_px = 400;
        int margin_x_px = 20;
        int margin_y_px = 20;
    } window;
    struct {
        float width_px = 30;
        float height_px = 20;
    } unit;
    struct {
        int margin_x_px = 0;
        int margin_y_px = 0;
    } job;
    struct {
        const float margin_x_u = -1.0/30;
        const float margin_y_u = -2.0/20;
        const float width_u = 2.0/30;
        const float height_u = 24.0/20;
    } deadline;
    struct {
        const float width_u = 4.0/30;
        const float height_u = 1;
        const float spacing_u = 2.0/30;
    } color_deadline;
    const int schedule_offset_y_u = 3;
    struct {
        const int offset_y_u = 10;
        int width_u = 0; // this gets computed later
        const int height_u = 1;
        const float grid_height_big_u = 1.0/2;
        const float grid_height_small_u = 1.0/6;
        const float poi_max_height_u = 2.0/3;
        const float poi_width_u = 1.0/3;
    } player;
} config;

struct job_rect {
    SDL_Rect r;
    int job_id;
    bool visible;
};

static int n_jobs;
static int n_schedules;
static std::vector<struct job_rect> schedule_render_positions;
static std::vector<struct job_rect> jobs_in_EDF_view_positions;
static std::vector<struct job_rect> jobs_in_EDF_view_render_positions;
static std::vector<struct job_rect> deadlines_render_positions;
static std::vector<struct job_rect> deadline_history_render_positions;
static std::map<int, std::vector<int>> deadlines;
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
static bool point_inside_rect(int x, int y, const SDL_Rect &r);

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
static void render_schedule(const struct job_rect &schedule);

/**
 * prepare rendering of jobs in EDF view
 */
static void render_jobs_in_EDF_view();

/**
 * prepare rendering of job deadlines in EDF view
 */
static void render_deadlines();

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

static void scale(SDL_Rect *r);

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
    SDL_Rect job_in_EDF_view = {
        offset,
        0,
        job.execution_time_estimate,
        1
    };
    SDL_Rect r = job_in_EDF_view;
    std::cout << "EDF SDL_Rect(" << r.x << ", " << r.y << ", " << r.w << ", " << r.h << ")"
              << std::endl;
    jobs_in_EDF_view_positions.push_back({job_in_EDF_view, job.id, true});

    if (deadlines.find(job.deadline) == deadlines.end()) {
        deadlines.insert({job.deadline, std::vector<int>()});
    }
    deadlines[job.deadline].push_back(job.id);
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
    }

    for (const Schedule &s: model->schedules) {

        /* create SDL rects for render positions */
        schedule_render_positions.push_back({{0,0,0,0},s.job_id,false});
        jobs_in_EDF_view_render_positions.push_back({{0,0,0,0},0,true});
        deadline_history_render_positions.push_back({{0,0,0,0},0,true});
    }
    /* create SDL rects for deadline render positions */
    for (unsigned i = 0; i < deadlines.size(); ++i) {
        deadlines_render_positions.push_back({{0,0,0,0},0,0});
    }
    config.player.width_u = std::max(offset,max_deadline);
    recompute_config();
}

void calculate_render_positions() {
    float timestamp = model->player.position;
    for (const Schedule &schedule: model->schedules) {
        /* precompute positions for job in schedule view */
        struct job_rect *s = &schedule_render_positions[schedule.id];
        /* TODO handle CFG and Recovery in other ways */
        int begin, execution_time;
        scheduler_type scheduler;
        std::tie(begin, scheduler, execution_time) = schedule.get_data_at_time(timestamp);
        s->visible = schedule.exists_at_time(timestamp) && scheduler == scheduler_type::ATLAS;
        s->r.x = begin;
        s->r.y = 10;
        s->r.w = execution_time;
        s->r.h = 1;
        //std::cout << "S   SDL_Rect(" << s->r.x << ", " << s->r.y << ", " << s->r.w << ", "
        //          << s->r.h << ")" << std::endl;
        scale(&s->r);
        s->r.y += u_to_px_h(config.schedule_offset_y_u);
    }

    for (int i = 0; i < n_jobs; ++i) {
        /* precompute positions for job in EDF view */
        struct job_rect *s = &jobs_in_EDF_view_render_positions[i];
        *s = jobs_in_EDF_view_positions[i];
        //std::cout << "EDF SDL_Rect(" << s->r.x << ", " << s->r.y << ", " << s->r.w << ", "
        //          << s->r.h << ")" << std::endl;
        scale(&s->r);
    }

    /* precompute positions for deadlines */
    int i = 0;
    for (std::pair<int, std::vector<int>> p: deadlines) {
        int deadline_position_x = u_to_px_w(p.first) + config.window.margin_x_px;
        SDL_Rect *r = &deadlines_render_positions[i].r;
        r->x = deadline_position_x + u_to_px_w(config.deadline.margin_x_u);
        r->y = config.window.margin_y_px + u_to_px_w(config.deadline.margin_y_u);
        r->w = u_to_px_w(config.deadline.width_u);
        r->h = u_to_px_h(config.deadline.height_u);

        int color_deadline_frame_width = p.second.size()
            * u_to_px_w(config.color_deadline.width_u + config.color_deadline.spacing_u)
            - u_to_px_w(config.color_deadline.spacing_u);

        int offset = -color_deadline_frame_width / 2;
        int it = color_deadline_frame_width / p.second.size();
        for (int id: p.second) {
            r = &deadline_history_render_positions[id].r;
            r->x = deadline_position_x + offset;
            r->y = config.window.margin_y_px
                + u_to_px_h(config.deadline.margin_y_u + config.deadline.height_u
                            + config.color_deadline.spacing_u);
            r->w = u_to_px_w(config.color_deadline.width_u);
            r->h = u_to_px_h(config.color_deadline.height_u);
            offset += it;
        }
        ++i;
    }
}

void render_vision() {
    calculate_render_positions();

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

    /* render current position on top of all */
    render_player_position();

    SDL_RenderPresent(renderer);
}

void render_schedule(const struct job_rect &schedule) {
    if (!schedule.visible) {
        return;
    }
    float modifier = (schedule.job_id == model->hovered_job) ? 1.0 : 0.9;
    set_color(schedule.job_id, modifier);
    SDL_RenderFillRect(renderer, &schedule.r);
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    SDL_RenderDrawRect(renderer, &schedule.r);
}

void render_jobs_in_schedule() {
    for (struct job_rect s: schedule_render_positions) {
        render_schedule(s);
    }
}

void render_jobs_in_EDF_view() {
    for (struct job_rect s: jobs_in_EDF_view_render_positions) {
        float modifier = (s.job_id == model->hovered_job) ? 1.0 : 0.9;
        set_color(s.job_id, modifier * 0.8);
        SDL_RenderFillRect(renderer, &s.r);
    }
}

void render_deadlines() {
    /* draw job history for each deadline */
    for (int i = 0; i < n_jobs; ++i) {
        float modifier = (i == model->hovered_job) ? 1.0 : 0.9;
        SDL_Rect r = deadline_history_render_positions[i].r;
        set_color(i, modifier);
        SDL_RenderFillRect(renderer, &r);
    }

    /* draw deadline in EDF view */
    for (unsigned i = 0; i < deadlines.size(); ++i) {
        SDL_Rect r = deadlines_render_positions[i].r;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &r);
    }
}

void render_player() {
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
    for (struct job_rect s: jobs_in_EDF_view_render_positions) {
        if (point_inside_rect(x, y, s.r)) {
            return s.job_id;
        }
    }
    for (struct job_rect s: schedule_render_positions) {
        if (point_inside_rect(x, y, s.r)) {
            return s.job_id;
        }
    }
    return -1;
}

bool point_inside_rect(int x, int y, const SDL_Rect &r) {
    return x > r.x && x < r.x + r.w
        && y > r.y && y < r.y + r.h;
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

static void scale(SDL_Rect *r) {
    r->x *= config.unit.width_px;
    r->x += config.window.margin_x_px;
    r->y *= config.unit.height_px;
    r->y += config.window.margin_y_px;
    r->w *= config.unit.width_px;
    r->h *= config.unit.height_px;
}
