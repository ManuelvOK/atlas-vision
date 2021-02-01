#include <plugins/atlas_plugin.h>

#include <fstream>
#include <string>

#include <util/parser.h>


AtlasPlugin::AtlasPlugin(): SDL_GUI::PluginBase("Atlas Plugin") {
    this->_command_line.register_flag("nosim", "n", "no_simulation");
    this->_command_line.register_positional("input");
}

AtlasModel *AtlasPlugin::build_atlas_model() const {
    std::string input = this->_command_line.get_positional("input");

    if (input == "") {
        std::cerr << "No input given." << std::endl;
        exit(1);
    }

    if (input == "-") {
        return this->atlas_model_from_stdin();
    }

    if (this->_command_line.get_flag("no_simulation")) {
        AtlasModel *model = this->atlas_model_from_file(input);
        model->_simulated = true;
        return model;
    }

    return this->atlas_model_from_file(input);
}

AtlasModel *AtlasPlugin::atlas_model_from_file(std::string path) const {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    AtlasModel *model = parse_file(&input_file);
    input_file.close();
    return model;
}

AtlasModel *AtlasPlugin::atlas_model_from_stdin() const {
    return parse_file(&std::cin);
}

AtlasModel *AtlasPlugin::parse_file(std::istream *input) const {
    Parser parser;
    AtlasModel *model = new AtlasModel();
    parser.parse(input, model);

    return model;
}
