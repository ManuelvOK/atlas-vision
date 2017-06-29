#include <input.h>

#include <SDL2/SDL.h>

static void handle_key_press(SDL_KeyboardEvent kb_event, struct input *input);
static void handle_key_release(SDL_KeyboardEvent kb_event);

void read_input(struct input *input) {
    SDL_Event event;
    while (0 != SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN: handle_key_press(event.key, input); break;
            case SDL_KEYUP: handle_key_release(event.key); break;
            default: break;
        }
    }
}

static void handle_key_press(SDL_KeyboardEvent kb_event, struct input *input) {
    if (kb_event.repeat) {
        return;
    }
    switch (kb_event.keysym.scancode) {
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_ESCAPE: input->quit = 1; break;
        default: break;
    }
}
static void handle_key_release(SDL_KeyboardEvent kb_event) {
    (void) kb_event;
    return;
}
