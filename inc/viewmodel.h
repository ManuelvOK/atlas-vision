#pragma once

#include <vector>
#include <map>

#include <SDL2/SDL.h>

#include <schedule_rect.h>

class Viewmodel {
  public:
    SDL_Window *window;
    SDL_Surface *player_surface;
    SDL_Surface *sidebar_surface;
    SDL_Surface *deadlines_and_submissions_surface;
    SDL_Surface *visibilities_surface;
    SDL_Surface *ATLAS_surface;
    SDL_Surface *recovery_surface;
    SDL_Surface *CFS_surface;
    SDL_Surface *dependencies_surface;
    SDL_Surface *event_logging_surface;

    int n_jobs;
    int n_schedules;
    std::vector<Schedule_rect> schedules;
    std::vector<Schedule_rect> EDF_schedules;
    std::vector<struct job_rect> deadlines_render_positions;
    std::vector<struct job_rect> deadline_history_render_positions;
    std::vector<struct job_rect> submission_render_positions;
    std::map<int, std::vector<int>> deadlines;
    std::map<int, std::vector<int>> submissions;
    std::vector<unsigned> colors;
};
