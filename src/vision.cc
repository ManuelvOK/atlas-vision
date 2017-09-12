#include <vision.h>

#include <controller.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

/**
 * SDL Window
 */
static SDL_Window *window;

/**
 * SDL Renderer
 */
static SDL_Renderer *renderer;

/* TODO: better get this dynamically done */
static int width;
static struct {
    struct {
        int width = 1000;
    } window;
    int margin_x = 20;
    int margin_y = 20;
    struct {
        int margin_x = 0;
        int margin_y = 0;
        int height = 20;
        int width = 30;
    } job;
    struct {
        int margin_x = -1;
        int margin_y = -2;
        int width = 2;
        int height = 24;
    } deadline;
    struct {
        int width = 4;
        int height = 20;
        int spacing = 2;
    } color_deadline;
    int schedule_offset_y = 0; // this gets computed later
    struct {
        int offset_x = 30;
        int offset_y = 700;
        int width = 600;
        int height = 30;
        int grid_height_big = 15;
        int grid_height_small = 5;
        int poi_max_height = 20;
        float poi_width = 1.0 / 3; // this is a factor for the unit width
    } player;
} config;

static int n_jobs;
static int n_schedules;
static std::vector<SDL_Rect> schedule_positions;
static std::vector<SDL_Rect> schedule_render_positions;
static std::vector<SDL_Rect> current_schedule_render_positions;
static std::vector<SDL_Rect> jobs_in_EDF_view_positions;
static std::vector<SDL_Rect> jobs_in_EDF_view_render_positions;
static std::vector<SDL_Rect> deadlines_render_positions;
static std::vector<SDL_Rect> deadline_history_render_positions;
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
 *
 * @param state
 *   application state
 */
static void calculate_render_positions(const struct state *state);

/**
 * check if point is inside a rendered job
 *
 * @param x
 *   x coordinate of point to check
 * @param y
 *   y coordinate of point to check
 * @param job
 *   id of job to check point for
 *
 * @return
 *   true if Point is inside one of the rendered job rects. Otherwise false
 */
static bool point_inside_rendered_job(int x, int y, int job);

/**
 * check if point is inside a rendered schedule
 *
 * @param x
 *   x coordinate of point to check
 * @param y
 *   y coordinate of point to check
 * @param schedule
 *   id of schedule to check point for
 *
 * @return
 *   true if Point is inside one of the rendered schedule rect. Otherwise false
 */
static bool point_inside_rendered_schedule(int x, int y, int schedule);
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
static bool point_inside_rect(int x, int y, SDL_Rect *r);

/**
 * prepare rendering of jobs in Schedule view
 *
 * @param state
 *   application state
 */
static void render_jobs_in_schedule(const struct state *state);

/**
 * prepare rendering of jobs in EDF view
 *
 * @param state
 *   application state
 */
static void render_jobs_in_EDF_view(const struct state *state);

/**
 * prepare rendering of job deadlines in EDF view
 *
 * @param state
 *   application state
 */
static void render_deadlines(const struct state *state);

/**
 * prepare rendering for everything that concerns the simulation playing
 *
 * @param state
 *   application state
 */
static void render_player(const struct state *state);

void exit_SDL_DestroyWindow(int status, void *window) {
    (void) status;
    SDL_DestroyWindow(static_cast<SDL_Window *>(window));
}

void exit_SDL_DestroyRenderer(int status, void *renderer) {
    (void) status;
    SDL_DestroyRenderer(static_cast<SDL_Renderer *>(renderer));
}

void init_graphics() {
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

void calculate_job_in_schedule(const struct state *state, const struct schedule &schedule) {
    //struct job job = state->jobs[schedule.job_id];
    SDL_Rect job_in_schedule = {
        schedule.begin,
        schedule.player_state, //schedule.core,
        schedule.execution_time,
        1,
    };
    schedule_positions.push_back(job_in_schedule);
}

void calculate_job_in_EDF_view(const struct job &job, int offset) {
    SDL_Rect job_in_EDF_view = {
        offset,
        0,
        job.execution_time_estimate,
        1
    };
    jobs_in_EDF_view_positions.push_back(job_in_EDF_view);

    if (deadlines.find(job.deadline) == deadlines.end()) {
        deadlines.insert({job.deadline, std::vector<int>()});
    }
    deadlines[job.deadline].push_back(job.id);
}

void calculate_vision(const struct state *state) {
    n_jobs = state->jobs.size();
    n_schedules = state->schedules.size();
    config.schedule_offset_y = config.job.height + config.color_deadline.height + 20;
    int offset = 0;
    for (const struct schedule &s: state->schedules) {
        struct job job = state->jobs[s.job_id];
        calculate_job_in_schedule(state, s);
        calculate_job_in_EDF_view(job, offset);
        offset += job.execution_time_estimate;

        /* create SDL rects for render positions */
        schedule_render_positions.push_back({0,0,0,0});
        jobs_in_EDF_view_render_positions.push_back({0,0,0,0});
        deadline_history_render_positions.push_back({0,0,0,0});
    }
    /* create SDL rects for deadline render positions */
    for (int i = 0; i < deadlines.size(); ++i) {
        deadlines_render_positions.push_back({0,0,0,0});
    }
    width = std::max(offset, state->jobs.back().deadline);
    config.job.width = config.window.width / width;
}

void calculate_render_positions(const struct state *state) {
    current_schedule_render_positions.clear();
    for (int i: state->player.states[state->player.current_state].schedules) {
        /* precompute positions for job in schedule view */
        SDL_Rect r = schedule_positions[i];
        r.x *= config.job.width;
        r.x += config.margin_x;
        r.y = 10;
        r.y *= config.job.height;
        r.y += config.margin_y + config.schedule_offset_y;
        r.w *= config.job.width;
        r.h *= config.job.height;
        current_schedule_render_positions.push_back(r);
    }

    for (int i = 0; i < n_schedules; ++i) {
        /* precompute positions for job in schedule view */
        SDL_Rect *r = &schedule_render_positions[i];
        *r = schedule_positions[i];
        r->x *= config.job.width;
        r->x += config.margin_x;
        r->y *= config.job.height;
        r->y += config.margin_y + config.schedule_offset_y;
        r->w *= config.job.width;
        r->h *= config.job.height;
    }

    for (int i = 0; i < n_jobs; ++i) {
        /* precompute positions for job in EDF view */
        SDL_Rect *r = &jobs_in_EDF_view_render_positions[i];
        *r = jobs_in_EDF_view_positions[i];
        r->x *= config.job.width;
        r->x += config.margin_x;
        r->y *= config.job.height;
        r->y += config.margin_y;
        r->w *= config.job.width;
        r->h *= config.job.height;
    }

    /* precompute positions for deadlines */
    int i = 0;
    for (std::pair<int, std::vector<int>> p: deadlines) {
        int deadline_position_x = p.first * config.job.width + config.margin_x;
        SDL_Rect *r = &deadlines_render_positions[i];
        r->x = deadline_position_x + config.deadline.margin_x;
        r->y = config.margin_y + config.deadline.margin_y;
        r->w = config.deadline.width;
        r->h = config.deadline.height;

        int color_deadline_frame_size = p.second.size()
            * (config.color_deadline.width + config.color_deadline.spacing)
            - config.color_deadline.spacing;

        int offset = -color_deadline_frame_size / 2;
        for (int id: p.second) {
            r = &deadline_history_render_positions[id];
            r->x = deadline_position_x + offset;
            r->y = config.margin_y + config.deadline.margin_y + config.deadline.height + config.color_deadline.spacing;
            r->w = config.color_deadline.width;
            r->h = config.color_deadline.height;
            offset += 6;
        }
        ++i;
    }
}

void render_vision(const struct state *state) {
    calculate_render_positions(state);

    /* init colors if not happend before */
    if (colors.empty()) {
        init_colors(state->jobs.size());
    }

    /* paint background white */
    SDL_SetRenderDrawColor(renderer, 170, 170, 170, 0);
    SDL_RenderClear(renderer);

    /* render job views */
    render_jobs_in_schedule(state);
    render_jobs_in_EDF_view(state);
    render_deadlines(state);

    /* render player */
    render_player(state);

    SDL_RenderPresent(renderer);
}

void render_jobs_in_schedule(const struct state *state) {
    /* TODO: this is copy paste skit. DONT */
    for (SDL_Rect r: current_schedule_render_positions) {
        set_color(0);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        SDL_RenderDrawRect(renderer, &r);

    }

    for (int i = 0; i < n_schedules; ++i) {
        struct schedule s = state->schedules[i];
        SDL_Rect r = schedule_render_positions[i];
        float modifier = (s.job_id == state->hovered_job) ? 1.0 : 0.9;
        set_color(s.job_id, modifier);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        SDL_RenderDrawRect(renderer, &r);
    }
}

void render_jobs_in_EDF_view(const struct state *state) {
    for (int i = 0; i < n_jobs; ++i) {
        SDL_Rect r = jobs_in_EDF_view_render_positions[i];
        float modifier = (i == state->hovered_job) ? 1.0 : 0.9;
        set_color(i, modifier * 0.8);
        SDL_RenderFillRect(renderer, &r);
    }
}

void render_deadlines(const struct state *state) {
    /* draw job history for each deadline */
    for (int i = 0; i < n_jobs; ++i) {
        float modifier = (i == state->hovered_job) ? 1.0 : 0.9;
        SDL_Rect r = deadline_history_render_positions[i];
        set_color(i, modifier);
        SDL_RenderFillRect(renderer, &r);
    }

    /* draw deadline in EDF view */
    for (int i = 0; i < deadlines.size(); ++i) {
        SDL_Rect r = deadlines_render_positions[i];
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &r);
    }
}

void render_player(const struct state *state) {
    /* render static player */
    SDL_Rect r;
    r.x = config.player.offset_x;
    r.y = config.player.offset_y;
    r.w = config.player.width;
    r.h = config.player.height;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &r);

    /* render unit markings */
    float unit = config.player.width * 1.0f / state->player.max_position;
    for (int i = 1; i < state->player.max_position; ++i) {
        float x = config.player.offset_x + unit * i;
        int y = config.player.offset_y + config.player.height;
        int height = config.player.grid_height_small;
        if (i % 5 == 0) {
            height = config.player.grid_height_big;
        }
        SDL_RenderDrawLine(renderer, x, y - height, x, y - 1);
    }

    /* render Points of interest */
    /* begin whith getting maximum value for submission count */
    int max_submissions = 0;
    for (struct player_state s: state->player.states) {
        max_submissions = std::max(max_submissions, s.n_submissions);
    }

    /* actually render new submissions into player overwiev */
    for (struct player_state s: state->player.states) {
        SDL_Rect r;
        r.x = config.player.offset_x + s.begin * unit + 1;
        r.y = config.player.offset_y + config.player.height - 1;
        r.w = unit * config.player.poi_width;
        r.h = -config.player.poi_max_height / max_submissions * s.n_submissions;
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
        SDL_RenderFillRect(renderer, &r);
    }


    /* render current position */
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    float x = config.player.offset_x + unit * state->player.position;
    float y = config.player.offset_y + 1;
    SDL_RenderDrawLine(renderer, x, y, x, y + config.player.height - 2);
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

int get_hovered_job(int x, int y, const struct state *state) {
    for (int i = 0; i < n_jobs; ++i) {
        if (point_inside_rendered_job(x, y, i)) {
            return i;
        }
    }
    for (int i = 0; i < n_schedules; ++i) {
        if (point_inside_rendered_schedule(x, y, i)) {
            return state->schedules[i].job_id;
        }
    }
    return -1;
}

bool point_inside_rendered_schedule(int x, int y, int schedule) {
    SDL_Rect *r_schedule = &schedule_render_positions[schedule];
    return point_inside_rect(x, y, r_schedule);
}

bool point_inside_rendered_job(int x, int y, int job) {
    SDL_Rect *r_EDF = &jobs_in_EDF_view_render_positions[job];
    SDL_Rect *r_deadline = &deadline_history_render_positions[job];
    return point_inside_rect(x, y, r_EDF)
        || point_inside_rect(x, y, r_deadline);
}

bool point_inside_rect(int x, int y, SDL_Rect *r) {
    return x > r->x && x < r->x + r->w
        && y > r->y && y < r->y + r->h;
}
