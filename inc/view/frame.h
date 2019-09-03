#pragma once
#include <vector>

#include <SDL2/SDL.h>

#include <model/model.h>
#include <util/attributable.h>
#include <view/viewmodel.h>
#include <view/drawable.h>
#include <view/positionable.h>


/**
 * Surface to display something at a certain position
 */
class Frame: public Positionable, public Attributable {
protected:
    Frame *_parent; /**< parent of frame in hierarchy. TODO: deprecated */
    Viewmodel *_viewmodel; /**< applications viewmodel. */
    int _offset_x; /**< horizontal offset in parent frame */
    int _offset_y; /**< vertical offset in parent frame */
    int _margin_left = 0; /**< left margin */
    int _margin_top = 0; /**< top margin */
    int _margin_right = 0; /**< right margin */
    int _margin_bottom = 0; /**< bottom margin */
    std::vector<Drawable *> _drawables; /**< List of drawables inside the Frame */
    std::vector<Frame *> _childs; /**< List of Frames inside the Frame */

    /**
     * update information of this frame non-recursively
     *
     * @param model
     *   applications model
     */
    virtual void update_this(const Model *model) = 0;

    /**
     * draw this frame non recursively
     * TODO: this should be done via _drawables
     *
     * @param renderer
     *   renderer to draw on
     * @param global_offset_x
     *   global horizontal offset
     * @param global_offset_y
     *   global vertical offset
     */
    virtual void draw_this(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     * @param offset_x
     *   horizontal offset inside parent frame
     * @param offset_y
     *   vertical offset inside parent frame
     * @param width
     *   frame width
     * @param height
     *   frame height
     */
    Frame(Frame *parent, Viewmodel *viewmodel, int offset_x, int offset_y, int width, int height)
        : Positionable(0,0,width,height), _parent(parent), _viewmodel(viewmodel),
          _offset_x(offset_x), _offset_y(offset_y) {}

    /**
     * Destructor
     */
    virtual ~Frame();

    /**
     * setter for left margin
     *
     * @param left
     *   left margin
     */
    void set_margin(int left);

    /**
     * setter for left and top margin
     *
     * @param left
     *   left margin
     * @param top
     *   top margin
     */
    void set_margin(int left, int top);

    /**
     * setter for left, top and right margin
     *
     * @param left
     *   left margin
     * @param top
     *   top margin
     * @param right
     *   right margin
     */
    void set_margin(int left, int top, int right);

    /**
     * setter for left, top, right and bottom margin
     *
     * @param left
     *   left margin
     * @param top
     *   top margin
     * @param right
     *   right margin
     * @param bottom
     *   bottom margin
     */
    void set_margin(int left, int top, int right, int bottom);

    /**
     * add a frame as child
     *
     * @param child
     *   frame to add
     */
    void add_child(Frame *child);

    /**
     * draw this frame recursively with shifted childs
     *
     * @param renderer
     *   renderer to draw on
     * @param local_offset_x
     *   TODO: some offset i guess
     * @param local_offset_y
     *   TODO: some offset i guess
     * @param shift_x
     *   horizontal shift of all childs
     * @param shift_y
     *   vertical shift of all childs
     * @param parent_clip_rect
     *   clipping rectangle of parent
     */
    virtual void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, int shift_x, int shift_y, SDL_Rect *parent_clip_rect) const;

    /**
     * draw this frame recursively
     *
     * @param renderer
     *   renderer to draw on
     * @param local_offset_x
     *   TODO: some offset i guess
     * @param local_offset_y
     *   TODO: some offset i guess
     * @param parent_clip_rect
     *   clipping rectangle of parent
     */
    void draw(SDL_Renderer *renderer, int local_offset_x, int local_offset_y, SDL_Rect *parent_clip_rect) const;

    /**
     * draw all drawables of this frame
     *
     * @param renderer
     *   the renderer to draw on
     * @param global offset_x
     *   horizontal offset of this frame in application
     * @param global offset_y
     *   vertical offset of this frame in application
     */
    virtual void draw_drawables(SDL_Renderer *renderer, int global_offset_x, int global_offset_y) const;

    /**
     * draw all child frames of this frame
     *
     * @param renderer
     *   the renderer to draw on
     * @param global offset_x
     *   horizontal offset of this frame in application
     * @param global offset_y
     *   vertical offset of this frame in application
     * @param clip_rect
     *   clipping rectangle of this or parent frame
     */
    virtual void draw_childs(SDL_Renderer *renderer, int global_offset_x, int global_offset_y, SDL_Rect *clip_rect) const;

    /**
     * update this frame recursively
     *
     * @param model
     *   applications model
     */
    void update(const Model *model);

    /**
     * getter for the position in application
     *
     * @returns
     *   position in application
     */
    Position global_position() const;
};
