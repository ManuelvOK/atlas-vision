#pragma once

#include <SDL2/SDL_ttf.h>

#include <SDL_GUI/inc/gui/primitives/text.h>

#include <models/message.h>

/** A Text that has different looks for being active or inactive.  */
class MessageText : public SDL_GUI::Drawable {
    SDL_Surface *_surface = nullptr;
    TTF_Font *_font;                /**< Font to use */
    std::string _text;              /**< text to display */
    SDL_Surface *_surface_active;   /**< prerendered text for being active */
    SDL_Surface *_surface_inactive; /**< prerendered text for being inactive */
public:
    /**
     * Constructor
     * @param message related simulation message
     * @param font font to use
     * @param width maximal width of textfield. Important for wrapping
     */
    MessageText(const std::string message, TTF_Font *font, int width, int offset_y);

    /** Destructor */
    ~MessageText();

    /** switch to surface for active text */
    void activate();

    /** switch to surface for inactive text */
    void deactivate();

    void draw(SDL_Renderer *renderer, SDL_GUI::Position position) const override;

    unsigned width() const override;

    unsigned height() const override;
};
