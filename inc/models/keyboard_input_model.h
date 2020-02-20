#pragma once

#include <SDL_GUI/inc/models/keyboard_input_model.h>

#include <array>

class KeyboardInputModel : public SDL_GUI::KeyboardInputModel {
    std::array<bool, 4> _current_states;
public:
    enum class Key {
        QUIT,
        PLAYER_TOGGLE,
        PLAYER_REWIND,
        PLAYER_FORWARDS,
        PLAYER_BACKWARDS,
    };

    void update();

    /**
     * set the state of a given key to be pressed
     * @param key The key to set as pressed
     */
    void set_pressed(Key key);

    /**
     * set the state of a given key to be released
     * @param key The key to set as released
     */
    void release(Key key);

    /**
     * get the state of a key
     * @param key the key to get the state of
     * @returns the state of the given key
     */
    bool is_pressed(Key key) const;
};
