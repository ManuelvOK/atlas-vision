#pragma once

#include <map>

#include <SDL2/SDL.h>

#include <input_keys.h>

/** config for keyboard input */
const std::map<SDL_Scancode, InputKey> keyboard_input_config = {
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
    {SDL_SCANCODE_L, InputKey::PLAYER_SCROLL_LEFT},
    {SDL_SCANCODE_H, InputKey::PLAYER_SCROLL_RIGHT},
    {SDL_SCANCODE_N, InputKey::REINIT},
};

/** config for window events */
const std::map<SDL_WindowEventID, InputKey> window_event_config = {
    {SDL_WINDOWEVENT_RESIZED, InputKey::RESIZED},
};

/** config for mouse input */
const std::map<Uint8, InputKey> mouse_input_config = {
    {SDL_BUTTON_LEFT, InputKey::CLICK},
    {SDL_BUTTON_RIGHT, InputKey::RIGHT_CLICK},
};
