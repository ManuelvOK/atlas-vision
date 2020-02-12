#include <SDL_GUI/inc/controllers/keyboard_input_controller.h>

class KeyboardInputController : public SDL_GUI::KeyboardInputController {
public:
    KeyboardInputController(bool *is_running) : SDL_GUI::KeyboardInputController(is_running) {}
};
