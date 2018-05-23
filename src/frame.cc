#include <frame.h>

#include <iostream>

Frame::~Frame() {
    for (Drawable *d: this->drawables) {
        delete d;
    }
    for (Frame *f: this->childs) {
        delete f;
    }
}

void Frame::set_margin(int left) {
    this->margin_left = left;
}
void Frame::set_margin(int left, int top) {
    this->margin_left = left;
    this->margin_top = top;
}
void Frame::set_margin(int left, int top, int right) {
    this->margin_left = left;
    this->margin_top = top;
    this->margin_right = right;
}
void Frame::set_margin(int left, int top, int right, int bottom) {
    this->margin_left = left;
    this->margin_top = top;
    this->margin_right = right;
    this->margin_bottom = bottom;
}

void Frame::add_child(Frame *child) {
    this->childs.push_back(child);
}

void Frame::draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y) const {
    int global_offset_x = local_offset_x + this->offset_x;
    int global_offset_y = local_offset_y + this->offset_y;
    SDL_Rect clip_rect = {
        global_offset_x - this->margin_left,
        global_offset_y - this->margin_top,
        this->width + this->margin_left + this->margin_right,
        this->height + this->margin_top + this->margin_bottom
    };
    SDL_RenderSetClipRect(renderer, &clip_rect);

    this->draw_this(renderer, global_offset_x, global_offset_y);
    /* draw everything for this frame */
    for (Drawable *drawable: this->drawables) {
        drawable->draw(renderer, global_offset_x, global_offset_y);
    }

    /* draw every child */
    for (Frame *child: this->childs) {
        child->draw(renderer, global_offset_x, global_offset_y);
    }
}
void Frame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    (void) renderer;
    (void) global_offset_x;
    (void) global_offset_y;
}

void Frame::update(const Model *model) {
    this->update_this(model);
    for (Frame *child: this->childs) {
        child->update(model);
    }
}

Position Frame::global_position() const {
    Position local_position(this->offset_x, this->offset_y);
    return (this->parent) ? this->parent->global_position() + local_position : local_position;
}