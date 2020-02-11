#include <iostream>

#include <SDL_GUI/inc/application.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <model/model.h>
#include <view/view.h>
#include <input.h>
#include <controller.h>

int main(int argc, char *argv[]) {
    SDL_GUI::Application application("Application");
    application.run();
    return 0;
}

#if 0
/**
 * Initialise SDL for video output
 */
void init_SDL();

void quit_SDL();

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
    struct input input = {0, 0, 0, 0, 0, {0, 0, 0, 0}, 1, {0, 0, 0}};
    while (model->_running) {
        read_input(&input, &view);
        handle_input(&input, &view);
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
    if (0 != TTF_Init()) {
        std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    atexit(quit_SDL);
}

void quit_SDL() {
    TTF_Quit();
    SDL_Quit();
}
#endif
