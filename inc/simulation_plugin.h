#pragma once

#include <fstream>

#include <SDL_GUI/application.h>
#include <SDL_GUI/controllers/input_controller.h>
#include <SDL_GUI/plugins/default_plugin.h>
#include <SDL_GUI/plugins/plugin_base.h>

#include <atlas/atlas_simulation_controller.h>
#include <atlas/atlas_simulation_model.h>
#include <atlas/atlas_view_controller.h>
#include <cbs/cbs_simulation_controller.h>
#include <cbs/cbs_simulation_model.h>
#include <cbs/cbs_view_controller.h>
#include <config/input_config.h>
#include <gui/interface_model.h>
#include <input/input_model.h>
#include <player/player_controller.h>
#include <player/player_model.h>
#include <simulation/schedule_change.h>
#include <simulation/simulation_model.h>


/** The Main Plugin for this application. It does the simulation visioning. */
class SimulationPlugin: public SDL_GUI::PluginBase {

    BaseSimulationModel *_simulation_model;
    PlayerModel *_player_model;

    template <typename T>
    T *build_model() const {
        std::string input = this->_command_line->get_positional("input");

        if (input == "") {
            std::cerr << "No input given." << std::endl;
            exit(1);
        }

        if (input == "-") {
            return this->model_from_stdin<T>();
        }

        T *model = this->model_from_file<T>(input);

        if (this->_command_line->get_flag("no-simulation")) {
            model->_simulated = true;
        }

        return model;
    }

    /**
     * Read simulation output from file
     * @param path file to read from
     * @return Model with all the parsed information
     */
    template <typename T>
    T *model_from_file(std::string path) const {
        std::ifstream input_file(path);
        if (not input_file.is_open()) {
            std::cerr << "Could not open file: " << path << std::endl;
            exit(EXIT_FAILURE);
        }
        T *model = parse_file<T>(&input_file);
        input_file.close();
        return model;
    }

    /**
     * Read simulation output from stdin
     * @return Model with all the parsed information
     */
    template <typename T>
    T *model_from_stdin() const {
        return parse_file<T>(&std::cin);
    }

    /**
     * Generate Atlas Model from an input stream
     * @param input input stream
     * @return Model with all the parsed information
     */
    template <typename T>
    T *parse_file(std::istream *input) const;
public:
    /** Constructor */
    SimulationPlugin(SDL_GUI::CommandLine *command_line);

    /**
     * Create all the needed Models, Controllers and Views
     * @tparam Ts List of already instantiated plugin types
     * @param app The application
     * @param plugins tuple of other plugins
     */
    template <typename ... Ts>
    void init(SDL_GUI::ApplicationBase *app, std::tuple<Ts...> *plugins) {
        /* headless things */
        this->_player_model = new PlayerModel();
        app->add_model(this->_player_model);

        CbsSimulationModel *cbs_model;
        CbsSimulationController *cbs_simulation_controller;

        AtlasSimulationModel *atlas_model;
        AtlasSimulationController *atlas_simulation_controller;

        if (this->_command_line->get_flag("simulate_cbs")) {
            cbs_model = this->build_model<CbsSimulationModel>();
            app->add_model(cbs_model);
            this->_simulation_model = cbs_model;
            this->_simulation_model->_only_simulation = app->is_headless();
            this->_simulation_model->_output_file = this->_command_line->get_option("output");

            cbs_simulation_controller =
                new CbsSimulationController(app, cbs_model, this->_player_model);
            app->add_controller(cbs_simulation_controller);
        } else {
            atlas_model = this->build_model<AtlasSimulationModel>();
            app->add_model(atlas_model);
            this->_simulation_model = atlas_model;
            this->_simulation_model->_only_simulation = app->is_headless();
            this->_simulation_model->_output_file = this->_command_line->get_option("output");

            atlas_simulation_controller =
                new AtlasSimulationController(app, atlas_model, this->_player_model);
            app->add_controller(atlas_simulation_controller);
        }

        if (app->is_headless()) {
            return;
        }

        /* Models */
        InputModel *input_model = new InputModel(InputState::ALL);
        app->add_model(input_model);

        InterfaceModel *interface_model = new InterfaceModel(this->_player_model);
        app->add_model(interface_model);

        /* Controllers */
        SDL_GUI::InputController<InputKey, InputState> *input_controller =
            new SDL_GUI::InputController<InputKey, InputState>(
                input_model, keyboard_input_config, window_event_config,
                mouse_input_config, InputKey::QUIT);
        app->add_controller(input_controller);

        SDL_GUI::DefaultPlugin &default_plugin = std::get<SDL_GUI::DefaultPlugin>(*plugins);
        SDL_GUI::InterfaceModel *default_interface_model = default_plugin.interface_model();

        if (this->_command_line->get_flag("simulate_cbs")) {
            CbsViewController *cbs_view_controller =
                new CbsViewController(app, cbs_model, interface_model, default_interface_model,
                                      input_model, this->_player_model);
            app->add_controller(cbs_view_controller);
        } else {
            AtlasViewController *atlas_view_controller =
                new AtlasViewController(app, atlas_model, interface_model, default_interface_model,
                                        input_model, this->_player_model);
            app->add_controller(atlas_view_controller);
        }


        PlayerController *player_controller =
            new PlayerController(this->_player_model, input_model, this->_simulation_model,
                                 interface_model, default_interface_model);
        app->add_controller(player_controller);
    }
};
