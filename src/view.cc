#include <view.h>

#include <iostream>

#include <vector>


static void exit_SDL_DestroyWindow(int status, void *window) {
    (void) status;
    SDL_DestroyWindow(static_cast<SDL_Window *>(window));
}

static void exit_SDL_DestroyRenderer(int status, void *renderer) {
    (void) status;
    SDL_DestroyRenderer(static_cast<SDL_Renderer *>(renderer));
}

View::View(const Model *model) {
    this->model = model;
    this->window = SDL_CreateWindow("visualisation", 0, 0, 640, 480, SDL_WINDOW_RESIZABLE);
    if (this->window == NULL) {
        std::cerr << "unable to create window: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyWindow, this->window);

    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderer == NULL) {
        std::cerr << "unable to create renderer: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyRenderer, this->renderer);

    this->viewmodel = new Viewmodel();

    //viewmodel->init_colors(model->jobs.size());

    this->create_frame_hierarchy();
}

void View::create_frame_hierarchy() {
    /* TODO: get magic offset values from config */
    this->window_frame = new WindowFrame(nullptr, this->viewmodel, 0, 0, 800, 600);
    PlayerFrame *player_frame = new PlayerFrame(this->window_frame, this->viewmodel, 10, 10, 590, 580);
    SidebarFrame *sidebar_frame = new SidebarFrame(this->window_frame, this->viewmodel, 600, 0, 190, 580);

    this->window_frame->add_child(player_frame);
    this->window_frame->add_child(sidebar_frame);

    SchedulerBackgroundFrame *scheduler_background_frame =
        new SchedulerBackgroundFrame(player_frame, this->viewmodel, 0, 100, 590, 110);
    PlayerGridFrame *player_grid_frame = new PlayerGridFrame(player_frame, this->viewmodel, 0, 0, 590, 580);
    DeadlineFrame *deadline_frame = new DeadlineFrame(player_frame, this->viewmodel, 0, 0, 590, 100);
    SchedulerFrame *ATLAS_frame = new SchedulerFrame(player_frame, this->viewmodel, 0, 100, 590, 30);
    ATLAS_frame->width = 100;
    ATLAS_frame->height = 20;
    SchedulerFrame *recovery_frame = new SchedulerFrame(player_frame, this->viewmodel, 0, 140, 590, 30);
    SchedulerFrame *CFS_frame = new SchedulerFrame(player_frame, this->viewmodel, 0, 180, 590, 30);
    VisibilityFrame *visibility_frame = new VisibilityFrame(player_frame, this->viewmodel, 0, 100, 590, 110);
    PlayerPositionFrame *player_position_frame =
        new PlayerPositionFrame(player_frame, this->viewmodel, 0, 0, 590, 580);

    player_frame->add_child(scheduler_background_frame);
    player_frame->add_child(player_grid_frame);
    player_frame->add_child(deadline_frame);
    player_frame->add_child(ATLAS_frame);
    player_frame->add_child(recovery_frame);
    player_frame->add_child(CFS_frame);
    player_frame->add_child(visibility_frame);
    player_frame->add_child(player_position_frame);

    DependencyFrame *dependency_frame = new DependencyFrame(sidebar_frame, this->viewmodel, 0, 0, 190, 400);
    EventFrame *event_frame = new EventFrame(sidebar_frame, this->viewmodel, 0, 400, 190, 170);

    sidebar_frame->add_child(dependency_frame);
    sidebar_frame->add_child(event_frame);
};

void View::render() {

    this->window_frame->draw(renderer, 0, 0);
    SDL_RenderPresent(renderer);

#if 0
    /* reset config_changed flag */
    config.changed = false;
#endif
}
