#include <view.h>

#include <vector>


void exit_SDL_DestroyWindow(int status, void *window) {
    (void) status;
    SDL_DestroyWindow(static_cast<SDL_Window *>(window));
}

void exit_SDL_DestroyRenderer(int status, void *renderer) {
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

    this->viewmodel = Viewmodel();

    this->create_frame_hierarchy();
}

void View::create_frame_hierarchy() {
    /* TODO: get magic offset values from config */
    this->window_frame = new WindowFrame(nullptr, &this->viewmodel, 0, 0);
    PlayerFrame *player_frame = new PlayerFrame(this->window_frame, &this->viewmodel, 10, 10);
    SidebarFrame *sidebar_frame = new SidebarFrame(this->window_frame, &this->viewmodel, 600, 0);

    this->window_frame.add_child(player_frame);
    this->window_frame.add_child(sidebar_frame);

    SchedulerBackgroundFrame *scheduler_background_frame =
        new SchedulerBackgroundFrame(player_frame, &this->viewmodel, 0, 100);
    PlayerGridFrame *player_grid_frame = new PlayerGridFrame(player_frame, &this->viewmodel, 0, 0);
    DeadlineFrame *deadline_frame = new DeadlineFrame(player_frame, &this->viewmodel, 0, 0);
    SchedulerFrame *ATLAS_frame = new SchedulerFrame(player_frame, &this->viewmodel, 0, 100);
    SchedulerFrame *recovery_frame = new SchedulerFrame(player_frame, &this->viewmodel, 0, 130);
    SchedulerFrame *CFS_frame = new SchedulerFrame(player_frame, &this->viewmodel, 0, 160);
    VisibilityFrame *visibility_frame = new VisbilityFrame(player_frame, &this->viewmodel, 0, 100);
    PlayerPositionFrame *player_position_frame =
        new PlayerPositionFrame(player_frame, &this->viewmodel, 0, 0);

    player_frame.add_child(scheduler_background_frame);
    player_frame.add_child(player_grid_frame);
    player_frame.add_child(deadline_frame);
    player_frame.add_child(ATLAS_frame);
    player_frame.add_child(recovery_frame);
    player_frame.add_child(CFS_frame);
    player_frame.add_child(visibility_frame);
    player_frame.add_child(player_position_frame);

    DependencyFrame *dependency_frame = new DependencyFrame(sidebar_frame, &this->viewmodel, 0, 0);
    EventFrame *event_frame = new EventFrame(sidebar_frame, &this->viewmodel, 0, 400);

    sidebar_frame.add_child(dependency_frame);
    sidebar_frame.add_child(event_frame);
};


void View::calculate() {
    /* TODO: this should be done recursively over the frame tree */
    this->window_frame.precompute();
#if 0
    this->viewmodel.n_jobs = this->model->jobs.size();
    this->viewmodel.n_schedules = this->model->schedules.size();

    /* the order of the job list can't be changed because the jobs are handled by id, which is the
     * position inside the list. That means we have to sort another list */
    std::vector<int> EDF_sorted_jobs;
    EDF_sorted_jobs.reserve(this->model->jobs.size());
    for (unsigned i = 0; i < this->model->jobs.size(); ++i) {
        EDF_sorted_jobs.push_back(i);
    }
    std::sort(EDF_sorted_jobs.begin(), EDF_sorted_jobs.end(), [](int a, int b) {
            return this->model->jobs.at(a).deadline < this->model->jobs.at(b).deadline;
        });

    /* calculate jobs in EDF view */
    /* TODO: is this needed any longer? */
#if 0
    int max_deadline = 0;
    int offset = 0;
    for (int i: EDF_sorted_jobs) {
        const Job &job = this->model->jobs.at(i);
        calculate_job_in_EDF_view(job, offset);
        offset += job.execution_time_estimate;
        max_deadline = std::max(max_deadline, job.deadline);
        viewmodel.deadline_history_render_positions.push_back({{0,0,0,0},0,true});
        viewmodel.submission_render_positions.push_back({{0, 0, 0, 0}, 0, true});
    }
#endif

    for (auto p: model->schedules) {
        const Schedule &s = p.second;
        /* create SDL rects for render positions */
        viewmodel.schedules.emplace_back(&config, s.job_id);
    }
    /* create SDL rects for deadline render positions */
    for (unsigned i = 0; i < viewmodel.deadlines.size(); ++i) {
        viewmodel.deadlines_render_positions.push_back({{0,0,0,0},0,0});
    }
    config.player.width_u = std::max(offset,max_deadline);
    recompute_config();
#endif
}

void View::render() {
    calculate_render_positions();
    calculate_submission_positions();

    /* init colors if not happend before */
    if (viewmodel.colors.empty()) {
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


