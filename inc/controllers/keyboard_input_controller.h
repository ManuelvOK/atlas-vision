#pragma once

#include <SDL_GUI/inc/controllers/keyboard_input_controller.h>

#include <models/keyboard_input_model.h>

class KeyboardInputController : public SDL_GUI::KeyboardInputController {
    KeyboardInputModel *_keyboard_input_model;
    void handle_key_press(SDL_KeyboardEvent kb_event) override;
    void handle_key_release(SDL_KeyboardEvent kb_event) override;
public:
    KeyboardInputController(bool *is_running, KeyboardInputModel *keyboard_input_model) :
        SDL_GUI::KeyboardInputController(is_running), _keyboard_input_model(keyboard_input_model) {}
};
