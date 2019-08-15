#pragma once
#include <vector>

#include <SDL2/SDL.h>

#include <model/model.h>
#include <util/attributable.h>
#include <view/viewmodel.h>
#include <view/drawable.h>
#include <view/positionable.h>


class Frame: public Positionable, public Attributable {
protected:
    Frame *parent;
    Viewmodel *viewmodel;
    int offset_x;
    int offset_y;
    int margin_left = 0;
    int margin_top = 0;
    int margin_right = 0;
    int margin_bottom = 0;
    std::vector<Drawable *> drawables;
    std::vector<Frame *> childs;

    virtual void update_this(const Model *model) = 0;
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
public:
    Frame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width, int height)
        : Positionable(0,0,width,height), parent(parent), viewmodel(viewmodel), offset_x(offset_x),
          offset_y(offset_y) {}

    virtual ~Frame();

    void set_margin(int left);
    void set_margin(int left, int top);
    void set_margin(int left, int top, int right);
    void set_margin(int left, int top, int right, int bottom);

    void add_child(Frame *child);
    virtual void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, int shift_x, int shift_y, SDL_Rect *parent_clip_rect) const;
    void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, SDL_Rect *parent_clip_rect) const;
    virtual void draw_drawables(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
    virtual void draw_childs(SDL_Renderer *renderer, int global_offset_x, int global_offset_y, SDL_Rect *clip_rect) const;
    void update(const Model *model);
    Position global_position() const;
};
