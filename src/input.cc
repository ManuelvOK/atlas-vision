#include <input.h>

#include <SDL2/SDL.h>

#include <vision.h>

/**
 * handle a pressed key
 *
 * @param kb_event
 *   keyboard event from SDL
 * @param input
 *   input struct to put data to
 */
static void handle_key_press(SDL_KeyboardEvent kb_event, struct input *input);

/**
 * handle a released key
 *
 * @param kb_event
 *   keyboard event from SDL
 */
static void handle_key_release(SDL_KeyboardEvent kb_event);

void handle_window_event(SDL_WindowEvent w_event, struct input *input);

/**
 * reset input struct to no input given
 *
 * @param input
 *   input struct to reset
 */
static void reset_input(struct input *input);

/**
 * to save whether mouse is clicked
 */
static int mouse_down = 0;

void read_input(struct input *input, const struct state *state) {
    reset_input(input);
    SDL_GetMouseState(&input->mouse_position_x, &input->mouse_position_y);

    SDL_Event event;
    while (0 != SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN: handle_key_press(event.key, input); break;
            case SDL_KEYUP: handle_key_release(event.key); break;
            case SDL_MOUSEBUTTONDOWN: mouse_down = 1; break;
            case SDL_MOUSEBUTTONUP: mouse_down = 0; break;
            case SDL_WINDOWEVENT: handle_window_event(event.window, input);
            default: break;
        }
        if (mouse_down) {
            input->player.position =
                position_in_player(input->mouse_position_x, input->mouse_position_y, state);
        }
    }
}

void handle_key_press(SDL_KeyboardEvent kb_event, struct input *input) {
    if (kb_event.repeat) {
        return;
    }
    switch (kb_event.keysym.scancode) {
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_ESCAPE: input->quit = 1; break;
        case SDL_SCANCODE_SPACE: input->player.toggle_play = 1; break;
        case SDL_SCANCODE_R: input->player.rewind = 1; break;
        default: break;
    }
}

void handle_key_release(SDL_KeyboardEvent kb_event) {
    (void) kb_event;
    return;
}

void handle_window_event(SDL_WindowEvent w_event, struct input *input) {
    switch(w_event.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            input->window.changed = 1;
            input->window.width = w_event.data1;
            input->window.height = w_event.data2;
            break;
        default:
            break;
    }
}

void reset_input(struct input *input) {
    input->player.toggle_play = 0;
    input->player.rewind = 0;
    input->player.position = -1;
    input->window.changed = 0;
}
