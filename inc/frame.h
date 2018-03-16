#include <vector>

#include <SDL2/SDL.h>

#include <model.h>
#include <viewmodel.h>

class Frame {
protected:
    Frame *parent;
    Viewmodel *viewmodel;
    int offset_x;
    int offset_y;
    int width;
    int height;
    std::vector<Drawable *> drawables;
    std::vector<Frame *> childs;

    virtual void update_this(const Model *model) = 0;
public:
    Frame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y)
        : parent(parent), viewmodel(viewmodel), offset_x(offset_x), offset_y(offset_y) {}
    virtual void add_child(Frame *child);
    virtual void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y) const;
    virtual void update(const Model *model);
};
