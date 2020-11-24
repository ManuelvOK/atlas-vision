#include <gui/message_text.h>

#include <sstream>

MessageText::MessageText(const std::string message, TTF_Font *font, int width, int offset_y) :
    Text(font, "") {
    /* TODO: get color from config */
    SDL_Color color = {255, 255, 255, 255};
    this->_surface_active = TTF_RenderText_Blended_Wrapped(this->_font, message.c_str(), color,
                                                           width);
    /* TODO: get color from config */
    color = {255, 255, 255, 180};
    this->_surface_inactive = TTF_RenderText_Blended_Wrapped(this->_font, message.c_str(), color,
                                                             width);
    SDL_FreeSurface(this->_surface);
    this->_surface = this->_surface_inactive;
    this->set_y(offset_y);
}

MessageText::~MessageText() {
    SDL_FreeSurface(this->_surface_active);
    SDL_FreeSurface(this->_surface_inactive);
    this->_surface = nullptr;
}

void MessageText::activate() {
    this->_surface = this->_surface_active;
}

void MessageText::deactivate() {
    this->_surface = this->_surface_inactive;
}
