#include <gui/message_text.h>

#include <sstream>

MessageText::MessageText(const std::string message, TTF_Font *font, int width, int offset_y) :
    Drawable("MessageText", {0, offset_y}), _font(font), _text(message) {

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
}

MessageText::~MessageText() {
    SDL_FreeSurface(this->_surface_active);
    SDL_FreeSurface(this->_surface_inactive);
}

void MessageText::activate() {
    this->_surface = this->_surface_active;
}

void MessageText::deactivate() {
    this->_surface = this->_surface_inactive;
}

void MessageText::draw(SDL_Renderer *renderer, SDL_GUI::Position position) const {
    SDL_Rect dstrect{position._x, position._y, this->_surface->w, this->_surface->h};
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, this->_surface);
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
}

unsigned MessageText::width() const {
    return this->_surface->w;
}

unsigned MessageText::height() const {
    return this->_surface->h;
}
