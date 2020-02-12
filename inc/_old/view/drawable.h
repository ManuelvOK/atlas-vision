#pragma once
#include <SDL2/SDL.h>

#include <view/positionable.h>

class Viewmodel;
class Model;
/**
 * Object that can be drawn on screen
 */
class Drawable : public Positionable {
protected:
    const Viewmodel *_viewmodel; /**< Viewmodel */
public:
    /**
     * Constructor
     *
     * @param viewmodel
     *   applications viewmodel
     */
    Drawable(const Viewmodel *viewmodel) : _viewmodel(viewmodel) {}

    /**
     * update data for drawing
     *
     * @param model
     *   applications model
     */
    virtual void update(const Model *model) { (void) model; }

    /**
     * draw this object
     *
     * @param renderer
     *   renderer to draw on
     * @param offset_x
     *   global horizontal offset
     * @param offset_y
     *   global vertical offset
     */
    virtual void draw(SDL_Renderer *renderer, int offset_x = 0, int offset_y = 0) const = 0;

    /**
     * check if object is visible at a given timestamp
     *
     * @param timestamp
     *   timestamp to check against
     * @returns
     *   True if object is visible at timestamp. False otherwise
     */
    virtual bool is_visible(int timestamp) const = 0;

    /**
     * default destructor
     */
    virtual ~Drawable() = default;
};
