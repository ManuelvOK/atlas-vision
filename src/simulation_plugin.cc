#include <simulation_plugin.h>

#include <fstream>
#include <string>

#include <util/parser.h>


SimulationPlugin::SimulationPlugin(): SDL_GUI::PluginBase("Atlas Plugin") {
    this->_command_line.register_flag("nosim", "n", "no_simulation");
    this->_command_line.register_flag("cbs", "c", "simulate_cbs");
    this->_command_line.register_positional("input");
}

AtlasSimulationModel *SimulationPlugin::build_atlas_model() const {
    std::string input = this->_command_line.get_positional("input");

    if (input == "") {
        std::cerr << "No input given." << std::endl;
        exit(1);
    }

    if (input == "-") {
        return this->atlas_model_from_stdin();
    }

    if (this->_command_line.get_flag("no_simulation")) {
        AtlasSimulationModel *model = this->atlas_model_from_file(input);
        model->_simulated = true;
        return model;
    }

    return this->atlas_model_from_file(input);
}

CbsSimulationModel *SimulationPlugin::build_cbs_model() const {
    return new CbsSimulationModel;
}

AtlasSimulationModel *SimulationPlugin::atlas_model_from_file(std::string path) const {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    AtlasSimulationModel *model = parse_file(&input_file);
    input_file.close();
    return model;
}

AtlasSimulationModel *SimulationPlugin::atlas_model_from_stdin() const {
    return parse_file(&std::cin);
}

AtlasSimulationModel *SimulationPlugin::parse_file(std::istream *input) const {
    Parser parser;
    AtlasSimulationModel *model = new AtlasSimulationModel();
    parser.parse(input, model);

    return model;
}
