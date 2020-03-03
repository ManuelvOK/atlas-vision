#pragma once

#include <fstream>

#include <SDL_GUI/inc/application.h>
#include <SDL_GUI/inc/models/input_model.h>

#include <input_keys.h>
#include <models/atlas_model.h>
#include <models/schedule_change.h>

class Application : public SDL_GUI::Application {
    SDL_GUI::InputModel<InputKey> *_input_model = nullptr;

    AtlasModel *_atlas_model;

    /**
     * update this->is_running variable
     */
    void update_running() override;

    /* TODO: put the following 4 functions into a factory or something */
    AtlasModel *atlas_model_from_file(std::string path) const;
    AtlasModel *atlas_model_from_stdin() const;
    AtlasModel *parse_file(std::istream *input) const;
    bool apply_schedule_change(AtlasModel *model, const ScheduleChange *change) const;
public:
    Application(std::string application_title, int argc, char *argv[]);

    /**
     * creates all the needed Models, Controllers and Views
     */
    void init_MVCs() override;
};

