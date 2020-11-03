#include <models/keyboard_input_model.h>

void KeyboardInputModel::update() {
    for (bool &state: this->_current_states) {
        state = false;
    }
}

void KeyboardInputModel::set_pressed(Key key) {
    this->_current_states[static_cast<unsigned>(key)] = true;
}

void KeyboardInputModel::release(Key key) {
    this->_current_states[static_cast<unsigned>(key)] = false;
}

bool KeyboardInputModel::is_pressed(Key key) const {
    return this->_current_states[static_cast<unsigned>(key)];
}
