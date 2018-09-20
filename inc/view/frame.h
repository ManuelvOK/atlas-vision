#pragma once
#include <vector>

#include <SDL2/SDL.h>

#include <model/model.h>
#include <view/viewmodel.h>
#include <view/drawable.h>

class Position {
public:
    int x;
    int y;

    Position(int x, int y) : x(x), y(y) {}
    Position operator+(Position &p) {
        return Position(this->x + p.x, this->y + p.y);
    }
};

class Frame {
protected:
    Frame *parent;
    Viewmodel *viewmodel;
    int offset_x;
    int offset_y;
    int shift_offset_x = 0;
    int shift_offset_y = 0;
    int margin_left = 0;
    int margin_top = 0;
    int margin_right = 0;
    int margin_bottom = 0;
    std::vector<Drawable *> drawables;
    std::vector<Frame *> childs;

    virtual void update_this(const Model *model) = 0;
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
public:
    int width;
    int height;

    Frame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width, int height)
        : parent(parent), viewmodel(viewmodel), offset_x(offset_x), offset_y(offset_y),
          width(width), height(height) {}

    virtual ~Frame();

    void set_margin(int left);
    void set_margin(int left, int top);
    void set_margin(int left, int top, int right);
    void set_margin(int left, int top, int right, int bottom);

    virtual void add_child(Frame *child);
    virtual void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y) const;
    virtual void update(const Model *model);
    virtual Position global_position() const;
    virtual void shift_x(int offset);
};
