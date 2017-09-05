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

/**
 * Data for rendering.
 *
 * this gets precomputed at the beginning and is used for rendering.
 *
 * @param n_jobs
 *   number of jobs
 * @param width
 *
 * @param window_width
 *
 */
static struct vision_state{
    int n_jobs;
    int width;
    int window_width = 1000;
    int margin_x = 20;
    int margin_y = 20;
    int job_margin_x = 0;
    int job_margin_y = 0;
    int job_height = 20;
    int job_width = 30;
    int deadline_margin_x = -1;
    int deadline_margin_y = -2;
    int deadline_width = 2;
    int deadline_height = 24;
    int color_deadline_width = 4;
    int color_deadline_height = 20;
    int color_deadline_spacing = 2;
    int schedule_offset_y;
    std::vector<SDL_Rect> jobs_in_schedule_position;
    std::vector<SDL_Rect> jobs_in_schedule_render_position;
    std::vector<SDL_Rect> jobs_in_EDF_view_position;
    std::vector<SDL_Rect> jobs_in_EDF_view_render_position;
    std::vector<SDL_Rect> deadlines_render_position;
    std::vector<SDL_Rect> deadline_history_render_position;
    std::map<int, std::vector<int>> deadlines;
    std::vector<unsigned> colors;
} vision_state;

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
void calculate_render_positions();

bool point_inside_rendered_job(int x, int y, int job);

bool point_inside_rect(int x, int y, SDL_Rect *r);

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
    struct job job = state->jobs[schedule.job_id];
    SDL_Rect job_in_schedule = {
        schedule.start,
        schedule.core,
        job.time,
        1,
    };
    vision_state.jobs_in_schedule_position.push_back(job_in_schedule);
}

void calculate_job_in_EDF_view(const struct job &job, int offset) {
    SDL_Rect job_in_EDF_view = {
        offset,
        0,
        job.time,
        1
    };
    vision_state.jobs_in_EDF_view_position.push_back(job_in_EDF_view);

    if (vision_state.deadlines.find(job.deadline) == vision_state.deadlines.end()) {
        vision_state.deadlines.insert({job.deadline, std::vector<int>()});
    }
    vision_state.deadlines[job.deadline].push_back(job.id);
}

void calculate_vision(const struct state *state) {
    vision_state.n_jobs = state->jobs.size();
    vision_state.schedule_offset_y = vision_state.job_height
                                     + vision_state.color_deadline_height
                                     + 20;
    int offset = 0;
    for (const struct schedule &s: state->schedules) {
        struct job job = state->jobs[s.job_id];
        calculate_job_in_schedule(state, s);
        calculate_job_in_EDF_view(job, offset);
        offset += job.time;
        /* create SDL rects for render positions */
        vision_state.jobs_in_schedule_render_position.push_back({0,0,0,0});
        vision_state.jobs_in_EDF_view_render_position.push_back({0,0,0,0});
        vision_state.deadline_history_render_position.push_back({0,0,0,0});
    }
    /* create SDL rects for deadline render positions */
    for (int i = 0; i < vision_state.deadlines.size(); ++i) {
        vision_state.deadlines_render_position.push_back({0,0,0,0});
    }
    vision_state.width = std::max(offset, state->jobs.back().deadline);
    vision_state.job_width = vision_state.window_width / vision_state.width;
}

void calculate_render_positions() {

    for (int i = 0; i < vision_state.n_jobs; ++i) {
        /* precompute positions for job in schedule view */
        SDL_Rect *r = &vision_state.jobs_in_schedule_render_position[i];
        *r = vision_state.jobs_in_schedule_position[i];
        r->x *= vision_state.job_width;
        r->x += vision_state.margin_x;
        r->y *= vision_state.job_height;
        r->y += vision_state.margin_y + vision_state.schedule_offset_y;
        r->w *= vision_state.job_width;
        r->h *= vision_state.job_height;

        /* precompute positions for job in EDF view */
        r = &vision_state.jobs_in_EDF_view_render_position[i];
        *r = vision_state.jobs_in_EDF_view_position[i];
        r->x *= vision_state.job_width;
        r->x += vision_state.margin_x;
        r->y *= vision_state.job_height;
        r->y += vision_state.margin_y;
        r->w *= vision_state.job_width;
        r->h *= vision_state.job_height;
    }

    /* precompute positions for deadlines */
    int i = 0;
    for (std::pair<int, std::vector<int>> p: vision_state.deadlines) {
        int deadline_position_x = p.first * vision_state.job_width + vision_state.margin_x;
        SDL_Rect *r = &vision_state.deadlines_render_position[i];
        r->x = deadline_position_x + vision_state.deadline_margin_x;
        r->y = vision_state.margin_y + vision_state.deadline_margin_y;
        r->w = vision_state.deadline_width;
        r->h = vision_state.deadline_height;

        int color_deadline_frame_size = p.second.size()
            * (vision_state.color_deadline_width + vision_state.color_deadline_spacing)
            - vision_state.color_deadline_spacing;

        int offset = -color_deadline_frame_size / 2;
        for (int id: p.second) {
            r = &vision_state.deadline_history_render_position[id];
            r->x = deadline_position_x + offset;
            r->y = vision_state.margin_y + vision_state.deadline_margin_y + vision_state.deadline_height + vision_state.color_deadline_spacing;
            r->w = vision_state.color_deadline_width;
            r->h = vision_state.color_deadline_height;
            offset += 6;
        }
        ++i;
    }
}

void render_vision(const struct state *state) {
    if (vision_state.colors.empty()) {
        init_colors(state->jobs.size());
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);

    for (int i = 0; i < vision_state.n_jobs; ++i) {
        /* draw job in schedule view */
        SDL_Rect r = vision_state.jobs_in_schedule_render_position[i];
        float modifier = (i == state->hovered_job) ? 1.0 : 0.9;
        set_color(i, modifier);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        SDL_RenderDrawRect(renderer, &r);

        /* draw job in EDF view */
        r = vision_state.jobs_in_EDF_view_render_position[i];
        set_color(i, modifier * 0.8);
        SDL_RenderFillRect(renderer, &r);

        /* draw deadline for job in EDF View */
        r = vision_state.deadline_history_render_position[i];
        set_color(i, modifier);
        SDL_RenderFillRect(renderer, &r);
    }

    for (int i = 0; i < vision_state.deadlines.size(); ++i) {
        /* draw deadline for job in EDF View */
        SDL_Rect r = vision_state.deadlines_render_position[i];
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &r);
    }

    SDL_RenderPresent(renderer);
}

void init_colors(int n_jobs) {
    std::vector<unsigned> c;
    for (int i = 0; i < n_jobs; ++i) {
        c.push_back((360 / (n_jobs)) * i);
    }
    int half = c.size() / 2;
    vision_state.colors.reserve(n_jobs);
    for (int i = 0; i < half; ++i) {
        vision_state.colors.push_back(c[half + i]);
        vision_state.colors.push_back(c[i]);
    }
    if (c.size() % 2 == 1) {
        vision_state.colors.push_back(c.back());
    }
}

void set_color(int job, float modifier) {
    float r = 0;
    float g = 0;
    float b = 0;

    HSV_to_RGB((float)vision_state.colors[job], 0.7f, 0.9f * modifier, &r, &g, &b);
    SDL_SetRenderDrawColor(renderer, r * 255, g * 255, b * 255, 255);
}

void HSV_to_RGB(float h, float s, float v,
                float *r, float *g, float *b) {

    int i;
    float f, p, q, t;
    if( s == 0 ) {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }
    h /= 60;            // sector 0 to 5
    i = floor( h );
    f = h - i;          // factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );
    switch( i ) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default:        // case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}

int get_hovered_job(int x, int y) {
    calculate_render_positions();
    for (int i = 0; i < vision_state.n_jobs; ++i) {
        if (point_inside_rendered_job(x, y, i)) {
            return i;
        }
    }
    return -1;
}

bool point_inside_rendered_job(int x, int y, int job) {
    SDL_Rect *r_schedule = &vision_state.jobs_in_schedule_render_position[job];
    SDL_Rect *r_EDF = &vision_state.jobs_in_EDF_view_render_position[job];
    SDL_Rect *r_deadline = &vision_state.deadline_history_render_position[job];
    return point_inside_rect(x, y, r_schedule)
        || point_inside_rect(x, y, r_EDF)
        || point_inside_rect(x, y, r_deadline);
}

bool point_inside_rect(int x, int y, SDL_Rect *r) {
    return x > r->x && x < r->x + r->w
        && y > r->y && y < r->y + r->h;
}
