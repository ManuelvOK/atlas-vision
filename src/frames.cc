#include <frames.h>

#include <iostream>

#include <drawables.h>

void WindowFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    /* paint background white */
    SDL_SetRenderDrawColor(renderer, 170, 170, 170, 0);
    SDL_RenderClear(renderer);

}

void WindowFrame::update_this(const Model *model) {

}

void PlayerFrame::update_this(const Model *model) {

}

void SchedulerBackgroundFrame::update_this(const Model *model) {

}

void SchedulerBackgroundFrame::draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const {
    SDL_Rect r = {global_offset_x, global_offset_y, this->width, 30};
    SDL_SetRenderDrawColor(renderer, 190, 190, 190, 0);
    SDL_RenderFillRect(renderer, &r);

    r.y += 40;
    SDL_SetRenderDrawColor(renderer, 210, 210, 210, 0);
    SDL_RenderFillRect(renderer, &r);

    r.y += 40;
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 0);
    SDL_RenderFillRect(renderer, &r);
}

void PlayerGridFrame::update_this(const Model *model) {

}

void DeadlineFrame::update_this(const Model *model) {

}

void SchedulerFrame::update_this(const Model *model) {

}

void VisibilityFrame::update_this(const Model *model) {

}

void PlayerPositionFrame::update_this(const Model *model) {

}

void SidebarFrame::update_this(const Model *model) {

}

void DependencyFrame::update_this(const Model *model) {

}

void EventFrame::update_this(const Model *model) {

}
