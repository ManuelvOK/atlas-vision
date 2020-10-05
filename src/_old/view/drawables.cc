#include <view/drawables.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <view/viewmodel.h>

Arrow::Arrow(const Viewmodel *viewmodel, const Job *job, std::array<short,9> arrow_coords_x, std::array<short,9> arrow_coords_y, int x, int y) :
    Drawable(viewmodel),
    _job(job), _arrow_coords_x(arrow_coords_x), _arrow_coords_y(arrow_coords_y), _x(x), _y(y) {
}

void Arrow::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    //constexpr int size = decltype(this->_x).size();
    short pos_x[std::tuple_size<decltype(this->_arrow_coords_x)>::value];
    short pos_y[std::tuple_size<decltype(this->_arrow_coords_x)>::value];
    for (unsigned j = 0; j < this->_arrow_coords_x.size(); ++j) {
        /* TODO: get rid of magic 5 */
        pos_x[j] = this->_arrow_coords_x[j] / 5.0 + this->_viewmodel->u_to_px_w(this->_x) + offset_x;
        /* TODO: y coord should go through viewmodel, too */
        pos_y[j] = this->_arrow_coords_y[j] / 5.0 + this->_y + offset_y;
    }
    filledPolygonRGBA(renderer, pos_x, pos_y, this->_arrow_coords_x.size(), this->_color._r,
                      this->_color._g, this->_color._b, 255);
}

bool Arrow::is_visible(int timestamp) const {
   return this->_job->_submission_time >= timestamp;
}

void Rect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    SDL_Rect r = this->_rect;
    r.x += offset_x;
    r.y += offset_y;

    this->_color.activate(renderer);
    SDL_RenderFillRect(renderer, &r);

    /* draw border */
    if (not this->_border) {
        return;
    }
    this->_border_color.activate(renderer);
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
    Rect(viewmodel), _schedule(schedule) {
    this->_border = true;
    this->_color = this->_viewmodel->get_color(schedule->_job_id);
    /* TODO: get rid of magic number */
    this->_border_color = RGB(110);
}

void ScheduleRect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    if (not this->_visible) {
        return;
    }
    Rect::draw(renderer, offset_x, offset_y);
}

bool ScheduleRect::is_visible(int timestamp) const {
    return this->_schedule->is_active_at_time(timestamp);
}

void ScheduleRect::recalculate_position() {
    this->_rect.x = this->_viewmodel->u_to_px_w(this->_begin);
    this->_rect.w = this->_viewmodel->u_to_px_w(this->_time);
    this->_rect.h = this->_viewmodel->u_to_px_h(1) - 1;

}

JobRect::JobRect(const Viewmodel *viewmodel, const Job *job, int offset_x, int offset_y) :
    Rect(viewmodel), _job(job) {
        this->_rect.x = offset_x;
        this->_rect.y = offset_y;
        this->_rect.w = this->_viewmodel->u_to_px_w(1000) - 1;
        this->_rect.h = this->_viewmodel->u_to_px_h(1) - 1;
        this->_color = this->_viewmodel->get_color(job->_id);
        /* TODO: get rid of magic number */
        this->_border_color = RGB(110);
}

void JobRect::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    Rect::draw(renderer, offset_x, offset_y);

}

bool JobRect::is_visible(int timestamp) const {
    (void) timestamp;
    return true;
}

void Line::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    this->_color.activate(renderer);
    SDL_RenderDrawLine(renderer, this->_begin_x + offset_x, this->_begin_y + offset_y,
                       this->_end_x + offset_x, this->_end_y + offset_y);
}

JobDependencyLine::JobDependencyLine(const Viewmodel *viewmodel, JobRect *r1, JobRect *r2) :
    Line(viewmodel), _r1(r1), _r2(r2) {
    this->_begin_x = this->_r1->_rect.x + this->_r1->_rect.w / 2;
    this->_begin_y = this->_r1->_rect.y;
    this->_end_x = this->_r2->_rect.x + this->_r2->_rect.w / 2;
    this->_end_y = this->_r2->_rect.y + this->_r2->_rect.h;

    /* TODO: get rid of magic number */
    this->_color = RGB(110);
}


bool Line::is_visible(int timestamp) const {
    (void) timestamp;
    return true;
}


Text::Text(Viewmodel *viewmodel, int offset_x, int offset_y) :
    Drawable(viewmodel), _offset_x(offset_x), _offset_y(offset_y) {
}

void Text::draw(SDL_Renderer *renderer, int offset_x, int offset_y) const {
    assert(this->_surface != nullptr);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, this->_surface);
    SDL_Rect dstrect{offset_x + this->_offset_x, offset_y + this->_offset_y, this->_surface->w, this->_surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
}

int Text::width() const {
    assert(this->_surface != nullptr);
    return this->_surface->w;
}

int Text::height() const {
    assert(this->_surface != nullptr);
    return this->_surface->h;
}

SimpleText::SimpleText(Viewmodel *viewmodel, std::string text, int offset_x, int offset_y) :
    Text(viewmodel, offset_x, offset_y), _text(text) {
    SDL_Color color = {255, 255, 255, 255};
    this->_surface = TTF_RenderText_Blended(this->_viewmodel->font, this->_text.c_str(), color);
}

SimpleText::~SimpleText() {
    SDL_FreeSurface(this->_surface);
}

void SimpleText::set_offset_x(int offset_x) {
    this->_offset_x = offset_x;
}

void SimpleText::set_offset_y(int offset_y) {
    this->_offset_y = offset_y;
}

bool SimpleText::is_visible(int timestamp) const {
    (void) timestamp;
    return true;
}

MessageText::MessageText(Viewmodel *viewmodel, const Message *message, int width, int offset_y) :
    Text(viewmodel, 0, offset_y), _message(message) {
    std::stringstream ss;
    ss << this->_message->_timestamp << ": " << this->_message->_message;
    /* TODO: get color from viewmodel */
    SDL_Color color = {255, 255, 255, 255};
    this->_surface_active = TTF_RenderText_Blended_Wrapped(this->_viewmodel->font, ss.str().c_str(), color, width);
    /* TODO: get color from viewmodel */
    color = {255, 255, 255, 180};
    this->_surface_inactive = TTF_RenderText_Blended_Wrapped(this->_viewmodel->font, ss.str().c_str(), color, width);
    this->_surface = this->_surface_inactive;
}

void MessageText::update(const Model *model) {
    if (model->_player._position >= this->_message->_timestamp) {
        this->_surface = this->_surface_active;
    } else {
        this->_surface = this->_surface_inactive;
    }
}

MessageText::~MessageText() {
    SDL_FreeSurface(this->_surface_active);
    SDL_FreeSurface(this->_surface_inactive);
}

bool MessageText::is_visible(int timestamp) const {
    (void) timestamp;
    return true;
}

