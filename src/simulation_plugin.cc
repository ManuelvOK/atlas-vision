#include <simulation_plugin.h>

#include <fstream>
#include <string>

#include <atlas/atlas_parser.h>
#include <cbs/cbs_parser.h>


SimulationPlugin::SimulationPlugin(): SDL_GUI::PluginBase("Atlas Plugin") {
    this->_command_line.register_flag("nosim", "n", "no_simulation");
    this->_command_line.register_flag("onlysim", "s", "only_simulation");
    this->_command_line.register_flag("cbs", "c", "simulate_cbs");
    this->_command_line.register_positional("input");
    this->_command_line.register_option("output", "o", "output");
}

template<>
AtlasSimulationModel *
SimulationPlugin::parse_file<AtlasSimulationModel>(std::istream *input) const {
    AtlasParser parser;
    AtlasSimulationModel *model = new AtlasSimulationModel();
    parser.parse(input, model);

    return model;
}

template<>
CbsSimulationModel *
SimulationPlugin::parse_file<CbsSimulationModel>(std::istream *input) const {
    CbsParser parser;
    CbsSimulationModel *model = new CbsSimulationModel();
    parser.parse(input, model);

    return model;
}
