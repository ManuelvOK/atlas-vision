#pragma once

#include <SDL2/SDL_ttf.h>

#include <SDL_GUI/inc/gui/primitives/text.h>

#include <models/message.h>

class MessageText : public SDL_GUI::Text {
    SDL_Surface *_surface_active; /**< prerendered text for being active */
    SDL_Surface *_surface_inactive; /**< prerendered text for being inactive */

public:
    /**
     * Constructor
     *
     * @param message
     *   related simulation message
     * @param width
     *   maximal width of textfield. Important for wrapping
     * @param offset_y
     *   vertical offset in frame
     */
    MessageText(const std::string message, TTF_Font *font, int width, int offset_y);

    /**
     * Destructor
     */
    ~MessageText();

    void activate();
    void deactivate();
};
