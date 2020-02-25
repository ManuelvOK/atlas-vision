#pragma once

#include <map>

#include <SDL2/SDL.h>

#include <input_keys.h>

const std::map<SDL_Scancode, InputKey> input_config = {
    {SDL_SCANCODE_Q, InputKey::QUIT},
    {SDL_SCANCODE_ESCAPE, InputKey::QUIT},
    {SDL_SCANCODE_SPACE, InputKey::PLAYER_TOGGLE},
    {SDL_SCANCODE_R, InputKey::PLAYER_REWIND},
    {SDL_SCANCODE_LEFT, InputKey::PLAYER_BACKWARDS},
    {SDL_SCANCODE_RIGHT, InputKey::PLAYER_FORWARDS},
};
