#include <view/frames.h>

#include <algorithm>
#include <iostream>

#include <view/drawables.h>

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

PlayerFrame::PlayerFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                         int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    this->_shift_x_max = this->_viewmodel->get_player_width_px();
}

void PlayerFrame::update_this(const Model *model) {
    /* TODO: mouse following while zooming */
    float player_position = model->_player._position;
    int player_position_offset = this->_viewmodel->u_to_px_w(player_position);
    int player_width_px = this->_viewmodel->get_player_width_px();

    /* CONTINUE:
     * this does not work as intended.*/
    /* follow player */
    /* TODO: get rid of magic 20 */
    if (player_position_offset - this->_shift_x > player_width_px) {
        std::cout << "ppo: " << player_position_offset
                  << "sft: " << this->_shift_x
                  << std::endl;
        this->_shift_x = player_position_offset - player_width_px;
    }

    if (this->_viewmodel->rescaled) {
        this->_shift_x = player_position_offset - (player_width_px / 2);
        this->repair_shift();
    }
}

void PlayerFrame::shift(int x) {
    this->_shift_x += x;
    this->repair_shift();
}

void PlayerFrame::repair_shift() {
    int player_width_px = this->_viewmodel->get_player_width_px();
    float scale = this->_viewmodel->unit_w / this->_viewmodel->unit_w_min;
    if (this->_shift_x < this->_shift_x_min) {
        this->_shift_x = this->_shift_x_min;
    } else if (this->_shift_x > this->_shift_x_max * scale - player_width_px) {
        this->_shift_x = this->_shift_x_max * scale - player_width_px;
    }
}

int PlayerFrame::get_shift_position() const {
    return this->_shift_x;
}

void PlayerFrame::draw_drawables(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    for (Drawable *drawable: this->_drawables) {
        drawable->draw(renderer, global_offset_x - this->_shift_x, global_offset_y);
    }
}

void PlayerFrame::draw_childs(SDL_Renderer *renderer, int global_offset_x, int global_offset_y, SDL_Rect *clip_rect) const {
    for (Frame *child: this->_childs) {
        child->draw(renderer, global_offset_x, global_offset_y, this->_shift_x, 0, clip_rect);
    }
}

SchedulerBackgroundFrame::SchedulerBackgroundFrame(Frame *parent, Viewmodel *viewmodel,
                                                   int offset_x, int offset_y, int width,
                                                   int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    /* add ATLAS background */
    SDL_Rect r = {0, this->_viewmodel->u_to_px_h(this->_viewmodel->config.schedule.ATLAS_offset_y_u),
                  this->_width, this->_viewmodel->u_to_px_h(1)};
    SchedulerRect *ATLAS_rect = new SchedulerRect(this->_viewmodel);
    ATLAS_rect->_rect = r;
    ATLAS_rect->_color = RGB(this->_viewmodel->config.schedule.ATLAS_grey);
    ATLAS_rect->_border = true;
    this->_drawables.push_back(ATLAS_rect);

    /* add recovery background */
    r.y = this->_viewmodel->u_to_px_h(this->_viewmodel->config.schedule.recovery_offset_y_u);
    SchedulerRect *recovery_rect = new SchedulerRect(this->_viewmodel);
    recovery_rect->_rect = r;
    recovery_rect->_color = RGB(this->_viewmodel->config.schedule.recovery_grey);
    recovery_rect->_border = true;
    this->_drawables.push_back(recovery_rect);

    /* add CFS background */
    r.y = this->_viewmodel->u_to_px_h(this->_viewmodel->config.schedule.CFS_offset_y_u);
    SchedulerRect *CFS_rect = new SchedulerRect(this->_viewmodel);
    CFS_rect->_rect = r;
    CFS_rect->_color = RGB(this->_viewmodel->config.schedule.CFS_grey);
    CFS_rect->_border = true;
    this->_drawables.push_back(CFS_rect);
}

void SchedulerBackgroundFrame::update_this(const Model *model) {
    (void) model;
}

void SchedulerBackgroundFrame::draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, int shift_x, int shift_y, SDL_Rect *parent_clip_rect) const {
    Frame::draw(renderer, local_offset_x, local_offset_y, parent_clip_rect);
}

PlayerGridFrame::PlayerGridFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                                 int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    /* add verticle lines */
    int player_width_px = this->_viewmodel->get_player_width_px();
    this->_n_lines = this->_viewmodel->px_to_u_w(this->_width) / player_width_px;
    for (int i = 0; i <= this->_n_lines; ++i) {
        int color = (i % 5 == 0) ? this->_viewmodel->config.player.grid_dark_grey : this->_viewmodel->config.player.grid_grey;
        Line *l = new Line(this->_viewmodel, this->_viewmodel->u_to_px_w(i * player_width_px), 0,
                           this->_viewmodel->u_to_px_w(i * player_width_px), this->_height);
        l->_color = RGB(color);
        this->_drawables.push_back(l);
    }

    for (std::pair<const int, std::vector<int>> &s: this->_viewmodel->submissions) {
        Line *l = new Line(this->_viewmodel, this->_viewmodel->u_to_px_w(s.first), 0,
                           this->_viewmodel->u_to_px_w(s.first), this->_height);
        l->_color = RGB(0, 255, 0);
        this->_drawables.push_back(l);
    }
}

void PlayerGridFrame::update_this(const Model *model) {
    (void) model;
    if (not this->_viewmodel->rescaled) {
        return;
    }
    this->rescale();
}

void PlayerGridFrame::rescale() {
    int i = 0;
    float player_width_px = this->_viewmodel->get_player_width_px();
    for (; i <= this->_n_lines; ++i) {
        Line *l = static_cast<Line *>(this->_drawables[i]);
        l->_begin_x = this->_viewmodel->u_to_px_w(i * player_width_px);
        l->_end_x = this->_viewmodel->u_to_px_w(i * player_width_px);
    }

    for (std::pair<const int, std::vector<int>> &s: this->_viewmodel->submissions) {
        Line *l = static_cast<Line *>(this->_drawables[i]);
        l->_begin_x = this->_viewmodel->u_to_px_w(s.first);
        l->_end_x = this->_viewmodel->u_to_px_w(s.first);
        ++i;
    }
}

DeadlineFrame::DeadlineFrame(const Model *model, Frame *parent, Viewmodel *viewmodel, int offset_x,
                             int offset_y, int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    /* get maximal submission count */
    for (std::pair<int, std::vector<int>> submissions: this->_viewmodel->submissions) {
        this->_max_n_submissions =
            std::max(this->_max_n_submissions, static_cast<unsigned>(submissions.second.size()));
    }

    /* get maximal deadline count */
    for (std::pair<int, std::vector<int>> deadlines: this->_viewmodel->deadlines) {
        this->_max_n_deadlines =
            std::max(this->_max_n_deadlines, static_cast<unsigned>(deadlines.second.size()));
    }

    /* create submissions */
    for (std::pair<int, std::vector<int>> submissions: this->_viewmodel->submissions) {
        int submission_position_x = submissions.first;

        /* TODO: get rid of magic number */
        int offset = this->_height - 7 * (submissions.second.size() - 1);
        for (int job: submissions.second) {
            const Job *job_ref = model->_jobs[job];
            SubmissionArrow *a =
                new SubmissionArrow(this->_viewmodel, job_ref, submission_position_x, offset - 1);
            a->_color = this->_viewmodel->get_color(job);
            this->_drawables.push_back(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }
    /* create deadlines */
    for (std::pair<int, std::vector<int>> deadlines: this->_viewmodel->deadlines) {
        int deadline_position_x = deadlines.first;

        /* TODO: get rid of magic number */
        int offset = 7 * (deadlines.second.size() - 1);
        for (int job: deadlines.second) {
            const Job *job_ref = model->_jobs[job];
            DeadlineArrow *a = new DeadlineArrow(this->_viewmodel, job_ref, deadline_position_x, offset - 1);
            a->_color = this->_viewmodel->get_color(job);
            this->_drawables.push_back(a);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
    }
}

void DeadlineFrame::update_this(const Model *model) {
    (void) model;
}


SchedulerFrame::~SchedulerFrame() {
    this->_drawables.clear();
}

void SchedulerFrame::update_this(const Model *model) {
    (void) model;

    this->_drawables.clear();
    for (ScheduleRect &r: this->_viewmodel->schedules) {
        if (r._scheduler == this->_scheduler) {
            this->_drawables.push_back(&r);
        }
    }
}

VisibilityFrame::VisibilityFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                                 int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    for (VisibilityLine &l: this->_viewmodel->visibilities) {
        this->_drawables.push_back(&l);
    }
}

VisibilityFrame::~VisibilityFrame() {
    this->_drawables.clear();
}

void VisibilityFrame::update_this(const Model *model) {
    (void) model;
}

PlayerPositionFrame::PlayerPositionFrame(Frame *parent, Viewmodel *viewmodel, int offset_x,
                                         int offset_y, int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    this->_player_line = new Line(this->_viewmodel, 0, 0, 0, this->_height);
    this->_player_line->_color = RGB(255, 0, 0);
    this->_drawables.push_back(this->_player_line);
}

void PlayerPositionFrame::update_this(const Model *model) {
    this->_player_line->_begin_x = this->_player_line->_end_x =
        this->_viewmodel->u_to_px_w(model->_player._position);
}

void SidebarFrame::update_this(const Model *model) {
    (void) model;
}

LegendFrame::LegendFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                         int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    std::vector<SimpleText *> index_texts;
    /* generate job id texts */
    for (const Job *j: *viewmodel->jobs) {
        SimpleText *t = new SimpleText(this->_viewmodel, std::to_string(j->_id), 0, 0);
        index_texts.push_back(t);
    }
    /* align job id texts */
    auto max_width_text = std::max_element(index_texts.begin(), index_texts.end(),
            [](Text *t1, Text *t2){ return t1->width() < t2->width(); });
    int max_width = (*max_width_text)->width();

    for (SimpleText *t: index_texts) {
        t->set_offset_x(max_width - t->width());
    }
    /* generate job rects */
    int job_offset_y = 0;
    int i = 0;
    for (const Job *j: *viewmodel->jobs) {
        JobRect *r = new JobRect(this->_viewmodel, j, max_width, job_offset_y);
        /* directly insert job rects */
        this->_drawables.push_back(r);
        /* align index texts */
        index_texts[i]->set_offset_y(job_offset_y);
        job_offset_y += r->_rect.h;
        ++i;
    }
    /* insert index texts into drawables structure */
    for (Drawable *d: index_texts) {
        this->_drawables.push_back(d);
    }
}

void LegendFrame::update_this(const Model *model) {
    (void) model;
}

DependencyFrame::DependencyFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y,
                                 int width, int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    std::map<int, std::vector<const Job *>> dependency_ordered_jobs;
    std::vector<JobRect *> job_rects;
    for (const Job *j: *viewmodel->jobs) {
        dependency_ordered_jobs[j->_dependency_level].push_back(j);
        JobRect *r = new JobRect(this->_viewmodel, j, 0, 0);
        job_rects.push_back(r);
    }
    for (std::pair<int, std::vector<const Job *>> jobs: dependency_ordered_jobs) {
        int i = 0;
        for (const Job * job: jobs.second) {
            JobRect *r = job_rects[job->_id];
            /* TODO: get rid of magic numbers */
            r->_rect.x = i * 31;
            r->_rect.y = jobs.first * 31;
            this->_drawables.push_back(r);
            ++i;
        }
    }
    for (const Job *j: *viewmodel->jobs) {
        for (const Job *dependency: j->_known_dependencies) {
            JobDependencyLine *l = new JobDependencyLine(this->_viewmodel, job_rects[j->_id], job_rects[dependency->_id]);
            this->_drawables.push_back(l);
        }
    }
}

void DependencyFrame::update_this(const Model *model) {
    (void) model;
}

EventFrame::EventFrame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width,
                       int height) :
    Frame(parent, viewmodel, offset_x, offset_y, width, height) {
    int text_offset_y = 0;
    for (Message *m: viewmodel->messages) {
        MessageText *message_text = new MessageText(this->_viewmodel, m, width, text_offset_y);
        text_offset_y += message_text->height();
        this->_drawables.push_back(message_text);
    }
}

void EventFrame::update_this(const Model *model) {
    for (Drawable *d: this->_drawables) {
        d->update(model);
    }
}
