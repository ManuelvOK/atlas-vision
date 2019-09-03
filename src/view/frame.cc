#include <view/frame.h>

#include <iostream>

Frame::~Frame() {
    for (Drawable *d: this->_drawables) {
        delete d;
    }
    for (Frame *f: this->_childs) {
        delete f;
    }
}

void Frame::set_margin(int left) {
    this->_margin_left = left;
}
void Frame::set_margin(int left, int top) {
    this->_margin_left = left;
    this->_margin_top = top;
}
void Frame::set_margin(int left, int top, int right) {
    this->_margin_left = left;
    this->_margin_top = top;
    this->_margin_right = right;
}
void Frame::set_margin(int left, int top, int right, int bottom) {
    this->_margin_left = left;
    this->_margin_top = top;
    this->_margin_right = right;
    this->_margin_bottom = bottom;
}

void Frame::add_child(Frame *child) {
    this->_childs.push_back(child);
}

void Frame::draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, int shift_x, int shift_y, SDL_Rect *parent_clip_rect) const {
    this->draw(renderer, local_offset_x - shift_x, local_offset_y - shift_y, parent_clip_rect);
}

void Frame::draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, SDL_Rect *parent_clip_rect) const {
    int parent_clip_x = (parent_clip_rect) ? parent_clip_rect->x : 0;
    int parent_clip_y = (parent_clip_rect) ? parent_clip_rect->y : 0;
    /* TODO: Inherit clip rect from parent */
    int clip_rect_x = std::max(local_offset_x + this->_offset_x, parent_clip_x);
    int clip_rect_y = std::max(local_offset_y + this->_offset_y, parent_clip_y);
    SDL_Rect clip_rect = {
        clip_rect_x - this->_margin_left,
        clip_rect_y - this->_margin_top,
        this->_width + this->_margin_left + this->_margin_right,
        this->_height + this->_margin_top + this->_margin_bottom
    };
    SDL_RenderSetClipRect(renderer, &clip_rect);

    int global_offset_x = local_offset_x + this->_offset_x;
    int global_offset_y = local_offset_y + this->_offset_y;

    this->draw_this(renderer, global_offset_x, global_offset_y);
    /* draw everything for this frame */
    this->draw_drawables(renderer, global_offset_x, global_offset_y);
    /* draw every child */
    this->draw_childs(renderer, global_offset_x, global_offset_y, &clip_rect);
}

void Frame::draw_drawables(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    for (Drawable *drawable: this->_drawables) {
        drawable->draw(renderer, global_offset_x, global_offset_y);
    }
}

void Frame::draw_childs(SDL_Renderer *renderer, int global_offset_x, int global_offset_y, SDL_Rect *clip_rect) const {
    for (Frame *child: this->_childs) {
        child->draw(renderer, global_offset_x, global_offset_y, clip_rect);
    }
}

void Frame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    (void) renderer;
    (void) global_offset_x;
    (void) global_offset_y;
}

void Frame::update(const Model *model) {
    this->update_this(model);
    for (Frame *child: this->_childs) {
        child->update(model);
    }
}

Position Frame::global_position() const {
    Position local_position(this->_offset_x, this->_offset_y);
    return (this->_parent) ? this->_parent->global_position() + local_position : local_position;
}
