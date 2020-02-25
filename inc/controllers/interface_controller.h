#pragma once

#include <SDL_GUI/inc/controllers/interface_controller.h>

#include <models/interface_model.h>
#include <models/mouse_input_model.h>
#include <models/player_model.h>

class InterfaceController : public SDL_GUI::InterfaceController {
    const PlayerModel *_player_model;

    void init_this();
public:
    InterfaceController(const std::string template_file_path,
            InterfaceModel *interface_model, const MouseInputModel *mouse_input_model, const PlayerModel *player_model);
};
