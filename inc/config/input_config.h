#pragma once

#include <map>
#include <set>

#include <SDL2/SDL.h>

/** enum for all possible inputs */
enum class InputKey {
    QUIT,
    PLAYER_TOGGLE,
    PLAYER_REWIND,
    PLAYER_FORWARDS,
    PLAYER_BACKWARDS,
    PLAYER_ZOOM_IN,
    PLAYER_ZOOM_OUT,
    PLAYER_SCROLL_LEFT,
    PLAYER_SCROLL_RIGHT,
    PLAYER_SET_POSITION,
    PLAYER_DRAG,
    RESIZED,
    REINIT,
    SHIFT,
    MESSAGE_CLICK,
};

enum class InputState {
    ALL,
    IN_PLAYER,
    IN_MESSAGES,
};

/** config for keyboard input */
const std::map<InputState,
               std::map<std::set<SDL_Scancode>,
                        std::map<SDL_Scancode, InputKey>>> keyboard_input_config = {
    {InputState::ALL, {
        {{}, {
            {SDL_SCANCODE_Q, InputKey::QUIT},
            {SDL_SCANCODE_ESCAPE, InputKey::QUIT},
            {SDL_SCANCODE_SPACE, InputKey::PLAYER_TOGGLE},
            {SDL_SCANCODE_R, InputKey::PLAYER_REWIND},
            {SDL_SCANCODE_LEFT, InputKey::PLAYER_BACKWARDS},
            {SDL_SCANCODE_RIGHT, InputKey::PLAYER_FORWARDS},
            {SDL_SCANCODE_UP, InputKey::PLAYER_ZOOM_IN},
            {SDL_SCANCODE_DOWN, InputKey::PLAYER_ZOOM_OUT},
            {SDL_SCANCODE_K, InputKey::PLAYER_ZOOM_IN},
            {SDL_SCANCODE_J, InputKey::PLAYER_ZOOM_OUT},
            {SDL_SCANCODE_L, InputKey::PLAYER_FORWARDS},
            {SDL_SCANCODE_H, InputKey::PLAYER_BACKWARDS},
            {SDL_SCANCODE_N, InputKey::REINIT},
            {SDL_SCANCODE_LSHIFT, InputKey::SHIFT},
            {SDL_SCANCODE_RSHIFT, InputKey::SHIFT},
        }},
        {{SDL_SCANCODE_LSHIFT}, {
            {SDL_SCANCODE_L, InputKey::PLAYER_SCROLL_LEFT},
            {SDL_SCANCODE_H, InputKey::PLAYER_SCROLL_RIGHT},
        }},
        {{SDL_SCANCODE_RSHIFT}, {
            {SDL_SCANCODE_L, InputKey::PLAYER_SCROLL_LEFT},
            {SDL_SCANCODE_H, InputKey::PLAYER_SCROLL_RIGHT},
        }},
    }},
};

/** config for window events */
const std::map<SDL_WindowEventID, InputKey> window_event_config = {
    {SDL_WINDOWEVENT_RESIZED, InputKey::RESIZED},
};

/** config for mouse input */
const std::map<InputState,
               std::map<std::set<SDL_Scancode>,
                        std::map<Uint8, InputKey>>> mouse_input_config = {
    {InputState::ALL, {
    }},
    {InputState::IN_PLAYER, {
        {{}, {
            {SDL_BUTTON_LEFT, InputKey::PLAYER_DRAG},
            {SDL_BUTTON_RIGHT, InputKey::PLAYER_SET_POSITION},
        }},
    }},
    {InputState::IN_MESSAGES, {
        {{}, {
            {SDL_BUTTON_LEFT, InputKey::MESSAGE_CLICK},
        }},
    }},
};
