#include <controllers/keyboard_input_controller.h>

void KeyboardInputController::handle_key_press(SDL_KeyboardEvent kb_event) {
    switch (kb_event.keysym.scancode) {
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_ESCAPE:
            this->_keyboard_input_model->set_pressed(KeyboardInputModel::Key::QUIT);
            *this->_is_running = false;
            break;
        case SDL_SCANCODE_SPACE:
            this->_keyboard_input_model->set_pressed(KeyboardInputModel::Key::PLAYER_TOGGLE);
            break;
        case SDL_SCANCODE_R:
            this->_keyboard_input_model->set_pressed(KeyboardInputModel::Key::PLAYER_REWIND);
            break;
        case SDL_SCANCODE_LEFT:
            this->_keyboard_input_model->set_pressed(KeyboardInputModel::Key::PLAYER_BACKWARDS);
            break;
        case SDL_SCANCODE_RIGHT:
            this->_keyboard_input_model->set_pressed(KeyboardInputModel::Key::PLAYER_FORWARDS);
            break;
        default:
            break;
    }
}

void KeyboardInputController::handle_key_release(SDL_KeyboardEvent kb_event) {
    switch (kb_event.keysym.scancode) {
        case SDL_SCANCODE_SPACE:
            this->_keyboard_input_model->release(KeyboardInputModel::Key::PLAYER_TOGGLE);
            break;
        case SDL_SCANCODE_R:
            this->_keyboard_input_model->release(KeyboardInputModel::Key::PLAYER_REWIND);
            break;
        case SDL_SCANCODE_LEFT:
            this->_keyboard_input_model->release(KeyboardInputModel::Key::PLAYER_BACKWARDS);
            break;
        case SDL_SCANCODE_RIGHT:
            this->_keyboard_input_model->release(KeyboardInputModel::Key::PLAYER_FORWARDS);
            break;
        default:
            break;
    }
}
