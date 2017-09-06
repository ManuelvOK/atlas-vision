#include <input.h>

#include <SDL2/SDL.h>

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

/**
 * reset input struct to no input given
 *
 * @param input
 *   input struct to reset
 */
static void reset_input(struct input *input);

void read_input(struct input *input) {
    reset_input(input);
    SDL_Event event;
    while (0 != SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN: handle_key_press(event.key, input); break;
            case SDL_KEYUP: handle_key_release(event.key); break;
            default: break;
        }
    }
    SDL_GetMouseState(&input->mouse_position_x, &input->mouse_position_y);
}

static void handle_key_press(SDL_KeyboardEvent kb_event, struct input *input) {
    if (kb_event.repeat) {
        return;
    }
    switch (kb_event.keysym.scancode) {
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_ESCAPE: input->quit = 1; break;
        case SDL_SCANCODE_SPACE: input->toggle_play = 1; break;
        case SDL_SCANCODE_R: input->rewind = 1; break;
        default: break;
    }
}

static void handle_key_release(SDL_KeyboardEvent kb_event) {
    (void) kb_event;
    return;
}

static void reset_input(struct input *input) {
    input->toggle_play = 0;
    input->rewind = 0;
}
