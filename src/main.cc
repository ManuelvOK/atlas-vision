#include <iostream>

#include <SDL2/SDL.h>

#include <vision.h>
#include <input.h>
#include <controller.h>

void init_SDL();

enum magic_numbers {
    MAGIC_33 = 33
};
int main(int argc, char ** argv) {
    init_SDL();
    init_graphics();

    const struct state *state = init_state();
    if (argc > 1) {
        read_input_from_file(argv[1]);
    } else {
        read_input_from_stdin();
    }
    if (not check_state()) {
        exit(EXIT_FAILURE);
    }

    struct input input = {0};
    while (state->running) {
        read_input(&input);
        handle_input(&input);
        render_vision(state);
        SDL_Delay(MAGIC_33);
    }

    return 0;
}
void init_SDL() {
    if (0 != SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    atexit(SDL_Quit);
}
