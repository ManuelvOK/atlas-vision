#include <frames.h>

void WindowFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void WindowFrame::update_this(const Model *model) {

}

void DeadlineFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void DeadlineFrame::update_this(const Model *model) {

}

void DeadlineFrame::update_submission_positions() {
    for (std::pair<int, std::vector<int>> p: this->viewmodel.submissions) {
        int submission_position_x = u_to_px_w(p.first) + this->viewmodel.config.window.margin_x_px;

        /* TODO: get rid of magic number */
        int offset = -7 * (p.second.size() - 1);
        for (int job: p.second) {
            SDL_Rect *r = &this->viewmodel.submission_render_positions[job].r;
            r->x = submission_position_x;
            r->y = u_to_px_h(this->viewmodel.config.schedule.offset_y_u)
                   + this->viewmodel.config.window.margin_y_px - 1 + offset;
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
}

void VisibilityFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void VisibilityFrame::update_this(const Model *model) {

}

void SchedulerFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void SchedulerFrame::update_this(const Model *model) {

}

void SchedulerFrame::update_schedule_render_position(const Model *model, const Schedule &schedule) {
    float timestamp = model->player.position;
    /* precompute positions for job in schedule view */
    Schedule_rect *s = &this->viewmodel.schedules[schedule.id];

    int begin;
    float execution_time;
    scheduler_type scheduler;
    std::tie(begin, scheduler, execution_time) = schedule.get_data_at_time(timestamp);

    s->visible = schedule.exists_at_time(timestamp);
    s->x = begin;
    s->w = execution_time;
    s->h = 1;

    switch (scheduler) {
        case scheduler_type::ATLAS:
            s->y = this->viewmodel.config.schedule.offset_y_u
                + this->viewmodel.config.schedule.ATLAS_offset_y_u;
            break;
        case scheduler_type::recovery:
            s->y = this->viewmodel.config.schedule.offset_y_u
                + this->viewmodel.config.schedule.recovery_offset_y_u;
            break;
        case scheduler_type::CFS:
            s->y = this->viewmodel.config.schedule.offset_y_u
                + this->viewmodel.config.schedule.CFS_offset_y_u;
            if (schedule.is_active_at_time(timestamp)) {
                s->w = timestamp - begin;
            }
            break;
    }
}
void SidebarFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void SidebarFrame::update_this(const Model *model) {

}

void DependencyFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void DependencyFrame::update_this(const Model *model) {

}

void EventFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {

}

void EventFrame::update_this(const Model *model) {

}
