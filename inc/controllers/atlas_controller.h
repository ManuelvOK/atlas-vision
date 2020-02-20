#pragma once

#include <SDL_GUI/inc/controllers/controller_base.h>

#include <models/atlas_model.h>

class AtlasController : public SDL_GUI::ControllerBase {
    AtlasModel *_atlas_model;
public:
    AtlasController(AtlasModel *atlas_model) : _atlas_model(atlas_model) {}
    void update();
};
