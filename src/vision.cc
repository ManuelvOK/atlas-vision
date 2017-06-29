#include <vision.h>

#include <controller.h>

#include <iostream>
#include <algorithm>
#include <vector>

static SDL_Window *window;
static SDL_Renderer *renderer;

static std::vector<unsigned> colors;

static void set_color(int job);
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

void render_vision(const struct state *state) {
    if (colors.empty()) {
        init_colors(state->jobs.size());
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    std::for_each(state->schedules.begin(), state->schedules.end(), [](struct schedule s){
        SDL_Rect r = {
            s.start * 20 + 10,
            s.core * 30 + 10,
            20 * s.time,
            20
        };
        set_color(s.job_id);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        SDL_RenderDrawRect(renderer, &r);
    });

    SDL_RenderPresent(renderer);
}

void init_colors(int n_jobs) {
    colors.reserve(n_jobs);
    for (int i = 0; i < n_jobs; ++i) {
        colors.push_back((360 / (n_jobs)) * i);
    }
}

void set_color(int job) {
    float r = 0;
    float g = 0;
    float b = 0;

    HSV_to_RGB((float)colors[job], 0.7f, 0.9f, &r, &g, &b);
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
