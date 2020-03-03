#pragma once

#include <map>

#include <SDL2/SDL.h>

#include <input_keys.h>

const std::map<SDL_Scancode, InputKey> keyboard_input_config = {
    {SDL_SCANCODE_Q, InputKey::QUIT},
    {SDL_SCANCODE_ESCAPE, InputKey::QUIT},
    {SDL_SCANCODE_SPACE, InputKey::PLAYER_TOGGLE},
    {SDL_SCANCODE_R, InputKey::PLAYER_REWIND},
    {SDL_SCANCODE_LEFT, InputKey::PLAYER_BACKWARDS},
    {SDL_SCANCODE_RIGHT, InputKey::PLAYER_FORWARDS},
    {SDL_SCANCODE_UP, InputKey::PLAYER_ZOOM_IN},
    {SDL_SCANCODE_DOWN, InputKey::PLAYER_ZOOM_OUT},
};

const std::map<SDL_WindowEventID, InputKey> window_event_config = {
    {SDL_WINDOWEVENT_RESIZED, InputKey::RESIZED},
};
