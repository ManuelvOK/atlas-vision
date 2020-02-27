#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>
#include <SDL_GUI/inc/models/keyboard_input_model.h>

#include <input_keys.h>
#include <models/atlas_model.h>
#include <models/interface_model.h>

class AtlasController : public SDL_GUI::ControllerBase {
    AtlasModel *_atlas_model;
    InterfaceModel *_interface_model;
    const SDL_GUI::KeyboardInputModel<InputKey> *_keyboard_input_model;
public:
    AtlasController(AtlasModel *atlas_model, InterfaceModel *interface_model, const SDL_GUI::KeyboardInputModel<InputKey> *keyboard_input_model);

    void init_this();
    void update();
};
