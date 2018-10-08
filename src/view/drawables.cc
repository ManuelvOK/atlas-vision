#include <view/drawables.h>

#include <iostream>
#include <algorithm>

#include <SDL2/SDL2_gfxPrimitives.h>

#include <view/viewmodel.h>

Arrow::Arrow(const Viewmodel *viewmodel, const Job *job, std::array<short,9> arrow_coords_x, std::array<short,9> arrow_coords_y, int x, int y) :
    Drawable(viewmodel),
    job(job), arrow_coords_x(arrow_coords_x), arrow_coords_y(arrow_coords_y), x(x), y(y) {
}

void Arrow::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    //constexpr int size = decltype(this->x).size();
    short pos_x[std::tuple_size<decltype(this->arrow_coords_x)>::value];
    short pos_y[std::tuple_size<decltype(this->arrow_coords_x)>::value];
    for (unsigned j = 0; j < this->arrow_coords_x.size(); ++j) {
        /* TODO: get rid of magic 5 */
        pos_x[j] = this->arrow_coords_x[j] / 5.0 + this->viewmodel->u_to_px_w(this->x) + offset_x;
        /* TODO: y coord should go through viewmodel, too */
        pos_y[j] = this->arrow_coords_y[j] / 5.0 + this->y + offset_y;
    }
    filledPolygonRGBA(renderer, pos_x, pos_y, this->arrow_coords_x.size(), this->color.r,
                      this->color.g, this->color.b, 255);
}

bool Arrow::is_visible(int timestamp) const {
   return this->job->submission_time >= timestamp;
}

void Rect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    SDL_Rect r = this->rect;
    r.x += offset_x;
    r.y += offset_y;

    this->color.activate(renderer);
    SDL_RenderFillRect(renderer, &r);

    /* draw border */
    if (not this->border) {
        return;
    }
    this->border_color.activate(renderer);
    SDL_RenderDrawRect(renderer, &r);
    /* since the drawn rect does not include the lower right point,
        * we have to draw it manually */
    SDL_RenderDrawPoint(renderer, r.x + r.w - 1, r.y + r.h - 1);
}

bool Rect::is_visible(int timestamp) const {
    (void) timestamp;
    return true;
}

ScheduleRect::ScheduleRect(const Viewmodel *viewmodel, const Schedule *schedule) :
    Rect(viewmodel),
    schedule(schedule) {
    this->border = true;
    /* TODO: get rid of magic number */
    this->border_color = RGB(110);
}

void ScheduleRect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    if (not this->visible) {
        return;
    }
    Rect::draw(renderer, offset_x, offset_y);
}

bool ScheduleRect::is_visible(int timestamp) const {
    return this->schedule->is_active_at_time(timestamp);
}

void ScheduleRect::recalculate_position() {
    this->rect.x = this->viewmodel->u_to_px_w(this->begin);
    this->rect.w = this->viewmodel->u_to_px_w(this->time);
    this->rect.h = this->viewmodel->u_to_px_h(1) - 1;

}

void Line::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    this->color.activate(renderer);
    SDL_RenderDrawLine(renderer, this->begin_x + offset_x, this->begin_y + offset_y,
                       this->end_x + offset_x, this->end_y + offset_y);
}

bool Line::is_visible(int timestamp) const {
    (void) timestamp;
    return true;
}

void VisibilityLine::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    if (not this->visible) {
        return;
    }
    Line::draw(renderer, offset_x, offset_y);
}

bool VisibilityLine::is_visible(int timestamp) const {
    return this->schedule->is_active_at_time(timestamp);
}
