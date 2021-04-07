#pragma once

#include <SDL_GUI/inc/application.h>
#include <SDL_GUI/inc/controllers/input_controller.h>
#include <SDL_GUI/inc/plugins/default_plugin.h>
#include <SDL_GUI/inc/plugins/plugin_base.h>
#include <SDL_GUI/inc/util/command_line.h>

#include <config/input_config.h>
#include <controllers/atlas_controller.h>
#include <controllers/player_controller.h>
#include <controllers/simulation_controller.h>
#include <models/atlas_model.h>
#include <models/input_model.h>
#include <models/interface_model.h>
#include <models/player_model.h>
#include <models/schedule_change.h>

/** The Main Plugin for this application. It does the simulation visioning. */
class AtlasPlugin: public SDL_GUI::PluginBase {
    SDL_GUI::CommandLine _command_line;

    AtlasModel *build_atlas_model() const;
    /**
     * Read simulation output from file
     * @param path file to read from
     * @return Model with all the parsed information
     */
    AtlasModel *atlas_model_from_file(std::string path) const;

    /**
     * Read simulation output from stdin
     * @return Model with all the parsed information
     */
    AtlasModel *atlas_model_from_stdin() const;

    /**
     * Generate Atlas Model from an input stream
     * @param input input stream
     * @return Model with all the parsed information
     */
    AtlasModel *parse_file(std::istream *input) const;
public:
    /** Constructor */
    AtlasPlugin();

    /**
     * Create all the needed Models, Controllers and Views
     * @tparam Ts List of already instantiated plugin types
     * @param app The application
     * @param previous tuple of already instantiated plugins
     * @param argc programs argc
     * @param argv[] programs argv
     */
    template <typename ... Ts>
    void init(SDL_GUI::ApplicationBase *app, std::tuple<Ts...> previous, int argc, char *argv[]) {
        this->_command_line.parse(argc, argv);

        /* Models */
        InputModel *input_model = new InputModel(InputState::ALL);
        app->add_model(input_model);

        PlayerModel *player_model = new PlayerModel();
        app->add_model(player_model);

        InterfaceModel *interface_model = new InterfaceModel(player_model);
        app->add_model(interface_model);

        AtlasModel *atlas_model = this->build_atlas_model();

        /* Controllers */
        SDL_GUI::InputController<InputKey, InputState> *input_controller =
            new SDL_GUI::InputController<InputKey, InputState>(
                input_model, keyboard_input_config, window_event_config,
                mouse_input_config);
        app->add_controller(input_controller);


        SDL_GUI::DefaultPlugin &default_plugin = std::get<SDL_GUI::DefaultPlugin>(previous);
        SDL_GUI::InterfaceModel *default_interface_model = default_plugin.interface_model();
        SimulationController *simulation_controller =
            new SimulationController(atlas_model, player_model, default_interface_model);
        app->add_controller(simulation_controller);

        AtlasController *atlas_controller = new AtlasController(app, atlas_model, interface_model,
                                                                default_interface_model,
                                                                input_model, player_model);
        app->add_controller(atlas_controller);

        PlayerController *player_controller = new PlayerController(player_model, input_model,
                                                                   atlas_model, interface_model,
                                                                   default_interface_model);
        app->add_controller(player_controller);
    }
};
