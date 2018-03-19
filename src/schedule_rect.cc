#include <schedule_rect.h>

#include <view_config.h>

SDL_Rect *Schedule_rect::render_position() {
    if (this->changed || this->config->changed) {
        this->calculate_render_position();
    }
    return &this->m_render_position;
}

void Schedule_rect::calculate_render_position() {
    SDL_Rect r;
    r.x = this->x * this->config->unit.width_px + this->config->window.margin_x_px;
    r.y = this->y * this->config->unit.height_px + this->config->window.margin_y_px;
    r.w = this->w * this->config->unit.width_px;
    r.h = this->h * this->config->unit.height_px;
    this->m_render_position = r;
    this->changed = false;
}
