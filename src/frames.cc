#include <frames.h>

#include <iostream>

#include <drawables.h>

void WindowFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    (void) global_offset_x;
    (void) global_offset_y;
    /* paint background white */
    SDL_SetRenderDrawColor(renderer, 170, 170, 170, 0);
    SDL_RenderClear(renderer);

}

void WindowFrame::update_this(const Model *model) {
    (void) model;
}

void PlayerFrame::update_this(const Model *model) {
    (void) model;
}

SchedulerBackgroundFrame::SchedulerBackgroundFrame(Frame *parent, Viewmodel *viewmodel,
                                                   int offset_x, int offset_y, int width,
                                                   int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    /* add ATLAS background */
    SDL_Rect r = {0, this->viewmodel->u_to_px_h(this->viewmodel->config.schedule.ATLAS_offset_y_u),
                  this->width, this->viewmodel->u_to_px_h(1)};
    SchedulerRect *ATLAS_rect = new SchedulerRect();
    ATLAS_rect->rect = r;
    ATLAS_rect->color = RGB(this->viewmodel->config.schedule.ATLAS_grey);
    ATLAS_rect->border = true;
    this->drawables.push_back(ATLAS_rect);

    /* add recovery background */
    r.y = this->viewmodel->u_to_px_h(this->viewmodel->config.schedule.recovery_offset_y_u);
    SchedulerRect *recovery_rect = new SchedulerRect();
    recovery_rect->rect = r;
    recovery_rect->color = RGB(this->viewmodel->config.schedule.recovery_grey);
    recovery_rect->border = true;
    this->drawables.push_back(recovery_rect);

    /* add CFS background */
    r.y = this->viewmodel->u_to_px_h(this->viewmodel->config.schedule.CFS_offset_y_u);
    SchedulerRect *CFS_rect = new SchedulerRect();
    CFS_rect->rect = r;
    CFS_rect->color = RGB(this->viewmodel->config.schedule.CFS_grey);
    CFS_rect->border = true;
    this->drawables.push_back(CFS_rect);
}

void SchedulerBackgroundFrame::update_this(const Model *model) {
    (void) model;
}

PlayerGridFrame::PlayerGridFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                                 int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    /* add verticle lines */
    int n_lines = this->viewmodel->px_to_u_w(this->width);
    for (int i = 0; i <= n_lines; ++i) {
        int color = (i % 5 == 0) ? this->viewmodel->config.player.grid_dark_grey : this->viewmodel->config.player.grid_grey;
        Line *l = new Line(this->viewmodel->u_to_px_w(i), 0, this->viewmodel->u_to_px_w(i), this->height);
        l->color = RGB(color);
        this->drawables.push_back(l);
    }
}

void PlayerGridFrame::update_this(const Model *model) {
    (void) model;
}

DeadlineFrame::DeadlineFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                             int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    /* get maximal submission count */
    for (std::pair<int, std::vector<int>> submissions: this->viewmodel->submissions) {
        this->max_n_submissions =
            std::max(this->max_n_submissions, static_cast<unsigned>(submissions.second.size()));
    }

    /* create submissions */
    for (std::pair<int, std::vector<int>> submissions: this->viewmodel->submissions) {
        int submission_position_x = this->viewmodel->u_to_px_w(submissions.first);

        /* TODO: get rid of magic number */
        int offset = this->height - 7 * (submissions.second.size() - 1);
        for (int job: submissions.second) {
            SubmissionArrow *a = new SubmissionArrow(submission_position_x, offset - 1);
            a->color = this->viewmodel->get_color(job);
            this->drawables.push_back(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
}

void DeadlineFrame::update_this(const Model *model) {
    (void) model;
}

void SchedulerFrame::update_this(const Model *model) {
    (void) model;
}

void VisibilityFrame::update_this(const Model *model) {
    (void) model;
}

PlayerPositionFrame::PlayerPositionFrame(Frame *parent, Viewmodel *viewmodel, int offset_x,
                                         int offset_y, int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    this->player_line = new Line(0, 0, 0, this->height);
    this->player_line->color = RGB(255, 0, 0);
    this->drawables.push_back(this->player_line);
}

void PlayerPositionFrame::update_this(const Model *model) {
    this->player_line->begin_x = this->player_line->end_x =
        this->viewmodel->u_to_px_w(model->player.position);
}

void SidebarFrame::update_this(const Model *model) {
    (void) model;
}

void DependencyFrame::update_this(const Model *model) {
    (void) model;
}

void EventFrame::update_this(const Model *model) {
    (void) model;
}
