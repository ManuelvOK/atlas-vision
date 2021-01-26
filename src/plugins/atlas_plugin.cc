#include <plugins/atlas_plugin.h>

#include <fstream>
#include <string>

#include <util/parser.h>


AtlasPlugin::AtlasPlugin(): SDL_GUI::PluginBase("Atlas Plugin") {
    this->_command_line.register_option("simulate", "s");
    this->_command_line.register_option("visualise", "v");
}

AtlasModel *AtlasPlugin::build_atlas_model() const {
    AtlasModel *model;
    std::string vision_input = this->_command_line.get_argument("visualise");
    std::string simulation_input = this->_command_line.get_argument("simulate");
    if (vision_input.size() and simulation_input.size()) {
        std::cerr << "Both simulation and visualisation input given. Using visualisation input"
                  << std::endl;
    }
    if (vision_input.size()) {
        model = this->atlas_model_from_file(vision_input);
        model->_simulated = true;
    } else if (simulation_input.size()) {
        model = this->atlas_model_from_file(simulation_input);
    } else {
        std::cerr << "No input given." << std::endl;
        exit(1);
    }
    /* sort jobs id wise */
    std::sort(model->_jobs.begin(), model->_jobs.end(),
              [](const Job *a, const Job *b) -> bool {return a->_id < b->_id;});

    return model;
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
