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

    this->renderer =
        SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderer == NULL) {
        std::cerr << "unable to create renderer: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyRenderer, this->renderer);

    this->viewmodel = new Viewmodel(this->model);

    //viewmodel->init_colors(model->jobs.size());

    this->create_frame_hierarchy();
}

View::~View() {
    delete this->window_frame;
}

void View::create_frame_hierarchy() {
    /* TODO: get magic offset values from config */
    this->window_frame = new WindowFrame(nullptr, this->viewmodel, 0, 0, 800, 600);
    this->player_frame = new PlayerFrame(this->window_frame, this->viewmodel, 20, 20, 580, 580);
    this->sidebar_frame = new SidebarFrame(this->window_frame, this->viewmodel, 600, 0, 190, 580);

    this->window_frame->add_child(player_frame);
    this->window_frame->add_child(sidebar_frame);

    this->scheduler_background_frame =
        new SchedulerBackgroundFrame(player_frame, this->viewmodel, 0, 100, 580, 110);
    this->player_grid_frame = new PlayerGridFrame(player_frame, this->viewmodel, 0, 0, 580, 580);
    this->deadline_frame = new DeadlineFrame(player_frame, this->viewmodel, 0, 0, 580, 100);
    this->deadline_frame->set_margin(10);
    this->ATLAS_frame =
        new SchedulerFrame(player_frame, this->viewmodel, 0, 100, 580, 20, SchedulerType::ATLAS);
    this->recovery_frame =
        new SchedulerFrame(player_frame, this->viewmodel, 0, 126, 580, 20, SchedulerType::recovery);
    this->CFS_frame =
        new SchedulerFrame(player_frame, this->viewmodel, 0, 152, 580, 20, SchedulerType::CFS);
    this->visibility_frame = new VisibilityFrame(player_frame, this->viewmodel, 0, 100, 580, 110);
    this->player_position_frame =
        new PlayerPositionFrame(player_frame, this->viewmodel, 0, 0, 580, 580);

    this->player_frame->add_child(scheduler_background_frame);
    this->player_frame->add_child(player_grid_frame);
    this->player_frame->add_child(deadline_frame);
    this->player_frame->add_child(ATLAS_frame);
    this->player_frame->add_child(recovery_frame);
    this->player_frame->add_child(CFS_frame);
    this->player_frame->add_child(visibility_frame);
    this->player_frame->add_child(player_position_frame);

    this->dependency_frame = new DependencyFrame(sidebar_frame, this->viewmodel, 0, 0, 190, 400);
    this->event_frame = new EventFrame(sidebar_frame, this->viewmodel, 0, 400, 190, 170);

    this->sidebar_frame->add_child(dependency_frame);
    this->sidebar_frame->add_child(event_frame);
}

void View::update_schedules() {
    float timestamp = this->model->player.position;
    for (std::pair<int, Schedule> s: this->model->schedules) {
        ScheduleRect &schedule_rect = this->viewmodel->schedules[s.first];

        int begin;
        float execution_time;
        SchedulerType scheduler;
        std::tie(begin, scheduler, execution_time) = s.second.get_data_at_time(timestamp);

        schedule_rect.begin = begin;
        if (scheduler == SchedulerType::CFS && s.second.is_active_at_time(timestamp)) {
            schedule_rect.time = timestamp - begin;
        } else {
            schedule_rect.time = execution_time;
        }
        schedule_rect.scheduler = scheduler;
        schedule_rect.visible = s.second.exists_at_time(timestamp);
        schedule_rect.recalculate_position();
    }
}

void View::update_visibilities() {
    float timestamp = this->model->player.position;
    for (unsigned i = 0; i < this->model->cfs_visibilities.size(); ++i) {
        Cfs_visibility visibility = this->model->cfs_visibilities[i];
        VisibilityLine &line = this->viewmodel->visibilities[i];
        if (not visibility.is_active_at_time(timestamp)) {
            line.visible = false;
            continue;
        }
        ScheduleRect schedule = this->viewmodel->schedules[visibility.schedule_id];
        line.begin_x = this->viewmodel->u_to_px_w(schedule.begin);
        /* TODO: get rid of magic number */
        line.begin_y = 10;
        line.end_x = this->viewmodel->u_to_px_w(timestamp);
        line.end_y = 62;
        line.visible = true;
    }
}

void View::render() {
    /* update viewmodel */
    this->update_schedules();
    this->update_visibilities();

    /* update frames */
    this->window_frame->update(this->model);

    /* render frames */
    this->window_frame->draw(renderer, 0, 0);
    SDL_RenderPresent(renderer);

#if 0
    /* reset config_changed flag */
    config.changed = false;
#endif
}

float View::position_in_player(int x, int y) const {
    (void) y;
    float shown_player_part =
        this->player_frame->width * 1.0 / this->viewmodel->u_to_px_w(this->model->player.max_position);
    float max_visible_player_position = shown_player_part * this->model->player.max_position;
    x -= this->player_frame->global_position().x;
    if (x < 0) {
        return 0;
    }
    if (x > this->player_frame->width) {
        return max_visible_player_position;
    }
    return x * 1.0f / this->player_frame->width * max_visible_player_position;
}
void View::rescale(float factor) {
    this->viewmodel->unit_w *= factor;
    this->viewmodel->rescaled = true;
}
