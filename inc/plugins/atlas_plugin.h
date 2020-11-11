#pragma once

#include <SDL_GUI/inc/application.h>
#include <SDL_GUI/inc/controllers/input_controller.h>
#include <SDL_GUI/inc/plugins/default_plugin.h>
#include <SDL_GUI/inc/plugins/plugin_base.h>

#include <input_keys.h>
#include <config/input_config.h>
#include <controllers/atlas_controller.h>
#include <controllers/player_controller.h>
#include <models/atlas_model.h>
#include <models/input_model.h>
#include <models/interface_model.h>
#include <models/player_model.h>
#include <models/player_view_model.h>
#include <models/schedule_change.h>

class AtlasPlugin: public SDL_GUI::PluginBase {
    AtlasModel *atlas_model_from_file(std::string path) const;
    AtlasModel *atlas_model_from_stdin() const;
    AtlasModel *parse_file(std::istream *input) const;
    bool apply_schedule_change(AtlasModel *model, const ScheduleChange *change) const;
public:
    AtlasPlugin(): SDL_GUI::PluginBase("Atlas Plugin") {}

    template <typename ... Ts>
    void init(SDL_GUI::ApplicationBase *app, std::tuple<Ts...> previous, int argc, char *argv[]) {
        /* Models */

        InputModel *input_model = new InputModel();
        app->add_model(input_model);

        PlayerModel *player_model = new PlayerModel();
        app->add_model(player_model);

        InterfaceModel *interface_model = new InterfaceModel(player_model);
        app->add_model(interface_model);

        PlayerViewModel *player_view_model = new PlayerViewModel();
        app->add_model(player_view_model);

        AtlasModel *atlas_model;
        if (argc > 1) {
            atlas_model = this->atlas_model_from_file(argv[1]);
        } else {
            atlas_model = this->atlas_model_from_stdin();
        }
        /* sort jobs id wise */
        std::sort(atlas_model->_jobs.begin(), atlas_model->_jobs.end(),
                  [](const Job *a, const Job *b) -> bool {return a->_id < b->_id;});

        /* Controllers */
        SDL_GUI::InputController<InputKey> *input_controller = new SDL_GUI::InputController<InputKey>(input_model, keyboard_input_config, window_event_config, mouse_input_config);
        app->add_controller(input_controller);


        SDL_GUI::DefaultPlugin &default_plugin = std::get<SDL_GUI::DefaultPlugin>(previous);
        SDL_GUI::InterfaceModel *default_interface_model = default_plugin.interface_model();
        AtlasController *atlas_controller = new AtlasController(app, atlas_model, interface_model, default_interface_model, input_model, player_model);
        app->add_controller(atlas_controller);

        PlayerController *player_controller = new PlayerController(player_model, player_view_model, input_model, atlas_model, interface_model, default_interface_model);
        app->add_controller(player_controller);

    }

};
