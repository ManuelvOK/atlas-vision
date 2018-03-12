#include <vector>

#include <SDL2/SDL.h>

#include <model.h>
#include <viewmodel.h>

class Frame {
private:
    Viewmodel *viewmodel;
    int offset_x;
    int offset_y;
    int width;
    int height;
    std::vector<Frame *> childs;

    virtual void draw_this(int global_offset_x, int global_offset_y) const = 0;
    virtual void update_this(const Model *model) = 0;
public:
    Frame(Viewmodel *viewmodel, int offset_x, int offset_y)
        : viewmodel(viewmodel), offset_x(offset_x), offset_y(offset_y) {}
    virtual void add_child(Frame *child);
    virtual void draw(int local_offset_x, int local_offset_y) const;
    virtual void update(const Model *model);
};
