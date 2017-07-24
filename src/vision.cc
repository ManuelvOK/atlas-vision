#include <vision.h>

#include <controller.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

static SDL_Window *window;
static SDL_Renderer *renderer;

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
    std::vector<SDL_Rect> jobs_in_schedule;
    std::vector<SDL_Rect> jobs_in_EDF_view;
    std::map<int, std::vector<int>> deadlines;
    std::vector<unsigned> colors;
} vision_state;

static void set_color(int job, float alpha = 1.0f);
static void init_colors(int n_jobs);
static void HSV_to_RGB(float h, float s, float v,
                       float *r, float *g, float *b);

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
    vision_state.jobs_in_schedule.push_back(job_in_schedule);
}

void calculate_job_in_EDF_view(const struct job &job, int offset) {
    SDL_Rect job_in_EDF_view = {
        offset,
        0,
        job.time,
        1
    };
    vision_state.jobs_in_EDF_view.push_back(job_in_EDF_view);

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
    }
    vision_state.width = std::max(offset, state->jobs.back().deadline);
    vision_state.job_width = vision_state.window_width / vision_state.width;
}
void render_vision(const struct state *state) {
    if (vision_state.colors.empty()) {
        init_colors(state->jobs.size());
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);

    /* draw schedule */
    for (int i = 0; i < vision_state.n_jobs; ++i) {
        SDL_Rect r = vision_state.jobs_in_schedule[i];
        r.x *= vision_state.job_width;
        r.x += vision_state.margin_x;
        r.y *= vision_state.job_height;
        r.y += vision_state.margin_y + vision_state.schedule_offset_y;
        r.w *= vision_state.job_width;
        r.h *= vision_state.job_height;
        set_color(i);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 1);
        SDL_RenderDrawRect(renderer, &r);
    }

    /* draw EDF view */
    for (int i = 0; i < vision_state.n_jobs; ++i) {
        SDL_Rect r = vision_state.jobs_in_EDF_view[i];
        r.x *= vision_state.job_width;
        r.x += vision_state.margin_x;
        r.y *= vision_state.job_height;
        r.y += vision_state.margin_y;
        r.w *= vision_state.job_width;
        r.h *= vision_state.job_height;
        set_color(i, 0.5f);
        SDL_RenderFillRect(renderer, &r);
    }
    for (std::pair<int, std::vector<int>> p: vision_state.deadlines) {
        int deadline_position_x = p.first * vision_state.job_width + vision_state.margin_x;
        SDL_Rect deadline = {
            deadline_position_x + vision_state.deadline_margin_x,
            vision_state.margin_y + vision_state.deadline_margin_y,
            vision_state.deadline_width,
            vision_state.deadline_height
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderFillRect(renderer, &deadline);

        int color_deadline_frame_size = p.second.size()
            * (vision_state.color_deadline_width + vision_state.color_deadline_spacing)
            - vision_state.color_deadline_spacing;

        int offset = -color_deadline_frame_size / 2;
        for (int id: p.second) {
            SDL_Rect job_deadline {
                deadline_position_x + offset,
                vision_state.margin_y + vision_state.deadline_margin_y + vision_state.deadline_height + vision_state.color_deadline_spacing,
                vision_state.color_deadline_width,
                vision_state.color_deadline_height
            };
            set_color(id);
            SDL_RenderFillRect(renderer, &job_deadline);
            offset += 6;
        }
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

void set_color(int job, float alpha) {
    float r = 0;
    float g = 0;
    float b = 0;

    HSV_to_RGB((float)vision_state.colors[job], 0.7f, 0.9f, &r, &g, &b);
    SDL_SetRenderDrawColor(renderer, r * 255, g * 255, b * 255, alpha);
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
