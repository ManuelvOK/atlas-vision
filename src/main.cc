#include <iostream>

#include <SDL2/SDL.h>

#include <model.h>
#include <view.h>
#include <input.h>
#include <controller.h>

/**
 * Initialise SDL for video output
 */
void init_SDL();

enum magic_numbers {
    MAGIC_33 = 33
};

int main(int argc, char *argv[]) {
    init_SDL();
    const Model *model = init_model();

    if (argc > 1) {
        read_input_from_file(argv[1]);
    } else {
        read_input_from_stdin();
    }
    if (not check_model()) {
        exit(EXIT_FAILURE);
    }

    View view(model);
    struct input input = {0, 0, 0, {0, 0, 0}, {0, 0, 0}};
    while (model->running) {
        read_input(&input);
        handle_input(&input);
        control();
        view.render();
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
