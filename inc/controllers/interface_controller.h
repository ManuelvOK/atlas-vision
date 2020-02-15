#pragma once

#include <SDL_GUI/inc/controllers/interface_controller.h>

#include <models/mouse_input_model.h>

class InterfaceController : public SDL_GUI::InterfaceController {
    void init_this();
public:
    InterfaceController(const std::string template_file_path,
            SDL_GUI::InterfaceModel *interface_model, const MouseInputModel *mouse_input_model);
};
