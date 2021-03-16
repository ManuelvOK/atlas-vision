#include <gui/message_text.h>

#include <sstream>

MessageText::MessageText(const std::string message, TTF_Font *font, int width,
                         SDL_GUI::Position position) :
    Drawable("MessageText", position), _font(font), _width(width), _text(message) {

    /* TODO: get color from config */
    SDL_Color color = {0, 0, 0, 255};
    this->_surface_active = TTF_RenderText_Blended_Wrapped(this->_font, message.c_str(), color,
                                                           width);
    /* TODO: get color from config */
    color = {0, 0, 0, 100};
    this->_surface_inactive = TTF_RenderText_Blended_Wrapped(this->_font, message.c_str(), color,
                                                             width);
    SDL_FreeSurface(this->_surface);
    this->change_surface(this->_surface_inactive);
}

MessageText::~MessageText() {
    SDL_FreeSurface(this->_surface_active);
    SDL_FreeSurface(this->_surface_inactive);
}

void MessageText::change_surface(SDL_Surface *surface) {
    this->_surface = surface;
    Positionable::_width = surface->w;
    this->_height = surface->h;
}

SDL_GUI::Drawable *MessageText::clone() const {
    return new MessageText(this->_text, this->_font, this->_width, this->_position);
}

void MessageText::activate() {
    this->change_surface(this->_surface_active);
}

void MessageText::deactivate() {
    this->change_surface(this->_surface_inactive);
}

void MessageText::draw(SDL_Renderer *renderer, SDL_GUI::Position position) const {
    SDL_Rect dstrect{position._x, position._y, this->_surface->w, this->_surface->h};
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, this->_surface);
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
}
