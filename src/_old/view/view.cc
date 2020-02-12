#include <view/view.h>

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
    this->_model = model;
    this->_window = SDL_CreateWindow("visualisation", 0, 0, 640, 480, SDL_WINDOW_RESIZABLE);
    if (this->_window == NULL) {
        std::cerr << "unable to create window: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyWindow, this->_window);

    this->_renderer =
        SDL_CreateRenderer(this->_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->_renderer == NULL) {
        std::cerr << "unable to create renderer: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    on_exit(exit_SDL_DestroyRenderer, this->_renderer);

    this->_viewmodel = new Viewmodel(this->_model);

    //viewmodel->init_colors(model->jobs.size());

    this->create_frame_hierarchy();
}

View::~View() {
    delete this->_window_frame;
}

void View::create_frame_hierarchy() {
    this->_frame_hierarchy.find_all_with_attribute("lol");
    /* TODO: get magic offset values from config */
    this->_window_frame = new WindowFrame(nullptr, this->_viewmodel, 0, 0, 800, 600);
    this->_player_frame = new PlayerFrame(this->_window_frame, this->_viewmodel, 20, 20, this->_viewmodel->config.player.width_px, this->_viewmodel->config.player.height_px);
    this->_sidebar_frame = new SidebarFrame(this->_window_frame, this->_viewmodel, 600, 0, 190, 580);

    this->_window_frame->add_child(this->_player_frame);
    this->_window_frame->add_child(this->_sidebar_frame);

    this->_scheduler_background_frame =
        new SchedulerBackgroundFrame(this->_player_frame, this->_viewmodel, 0, 100, 580, 110);
    this->_player_grid_frame = new PlayerGridFrame(this->_player_frame, this->_viewmodel, 0, 0, 580, 580);
    this->_deadline_frame = new DeadlineFrame(this->_model, this->_player_frame, this->_viewmodel, 0, 0, 580, 100);
    this->_deadline_frame->set_margin(10, 0, 10);
    this->_ATLAS_frame =
        new SchedulerFrame(this->_player_frame, this->_viewmodel, 0, 100, 580, 20, SchedulerType::ATLAS);
    this->_recovery_frame =
        new SchedulerFrame(this->_player_frame, this->_viewmodel, 0, 126, 580, 20, SchedulerType::recovery);
    this->_CFS_frame =
        new SchedulerFrame(this->_player_frame, this->_viewmodel, 0, 152, 580, 20, SchedulerType::CFS);
    this->_visibility_frame = new VisibilityFrame(this->_player_frame, this->_viewmodel, 0, 100, 580, 110);
    this->_player_position_frame =
        new PlayerPositionFrame(this->_player_frame, this->_viewmodel, 0, 0, 580, 580);

    this->_player_frame->add_child(this->_scheduler_background_frame);
    this->_player_frame->add_child(this->_player_grid_frame);
    this->_player_frame->add_child(this->_deadline_frame);
    this->_player_frame->add_child(this->_ATLAS_frame);
    this->_player_frame->add_child(this->_recovery_frame);
    this->_player_frame->add_child(this->_CFS_frame);
    this->_player_frame->add_child(this->_visibility_frame);
    this->_player_frame->add_child(this->_player_position_frame);

    this->_legend_frame = new LegendFrame(this->_sidebar_frame, this->_viewmodel, 0, 0, 40, 400);
    this->_dependency_frame = new DependencyFrame(this->_sidebar_frame, this->_viewmodel, 40, 0, 150, 400);
    this->_event_frame = new EventFrame(this->_sidebar_frame, this->_viewmodel, 0, 400, 190, 170);

    this->_sidebar_frame->add_child(this->_legend_frame);
    this->_sidebar_frame->add_child(this->_dependency_frame);
    this->_sidebar_frame->add_child(this->_event_frame);
}

void View::update_schedules() {
    int timestamp = this->_model->_player._position;
    for (std::pair<int, Schedule *> s: this->_model->_schedules) {
        ScheduleRect &schedule_rect = this->_viewmodel->schedules[s.first];

        int begin;
        int execution_time;
        SchedulerType scheduler;
        std::tie(begin, scheduler, execution_time) = s.second->get_data_at_time(timestamp);

        schedule_rect._begin = begin;
        if (scheduler == SchedulerType::CFS && s.second->is_active_at_time(timestamp)) {
            schedule_rect._time = timestamp - begin;
        } else {
            schedule_rect._time = execution_time;
        }
        schedule_rect._scheduler = scheduler;
        schedule_rect._visible = s.second->exists_at_time(timestamp);
        schedule_rect.recalculate_position();
    }
}

void View::update_visibilities() {
    int timestamp = this->_model->_player._position;
    for (unsigned i = 0; i < this->_model->_cfs_visibilities.size(); ++i) {
        CfsVisibility *visibility = this->_model->_cfs_visibilities[i];
        VisibilityLine &line = this->_viewmodel->visibilities[i];
        if (not visibility->is_active_at_time(timestamp)) {
            line._visible = false;
            continue;
        }
        ScheduleRect schedule = this->_viewmodel->schedules[visibility->_schedule_id];
        line._begin_x = this->_viewmodel->u_to_px_w(schedule._begin);
        /* TODO: get rid of magic number */
        line._begin_y = 10;
        line._end_x = this->_viewmodel->u_to_px_w(timestamp);
        line._end_y = 62;
        line._visible = true;
    }
}

void View::render() {
    /* update viewmodel */
    this->update_schedules();
    this->update_visibilities();

    /* update frames */
    this->_window_frame->update(this->_model);
    this->_viewmodel->rescaled = false;

    /* render frames */
    this->_window_frame->draw(this->_renderer, 0, 0, nullptr);
    SDL_RenderPresent(this->_renderer);

#if 0
    /* reset config_changed flag */
    config.changed = false;
#endif
}

int View::position_in_player(int x, int y) const {
    (void) y;
    float shown_player_part =
        this->_player_frame->_width * 1.0 / this->_viewmodel->u_to_px_w(this->_model->_player._max_position);
    int max_visible_player_position = shown_player_part * this->_model->_player._max_position;
    x -= this->_player_frame->global_position()._x -this->_player_frame->get_shift_position();
    if (x < 0) {
        return 0;
    }
    /* CONTINUE:
     * Clicking doesnt work with zooming. */
    if (x > this->_player_frame->_width) {
        return max_visible_player_position;
    }
    return x * 1.0f / this->_player_frame->_width * max_visible_player_position;
}

void View::rescale(float factor) {
    if (factor == 1.0) {
        return;
    }
    this->_viewmodel->unit_w = std::max(this->_viewmodel->unit_w * factor, this->_viewmodel->unit_w_min);
    this->_viewmodel->rescaled = true;
}

void View::shift_player(int offset) {
    this->_player_frame->shift(offset);
}
