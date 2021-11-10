#include <simulation_plugin.h>

#include <fstream>
#include <string>

#include <atlas/atlas_parser.h>
#include <cbs/cbs_parser.h>
#include <grub/grub_parser.h>


SimulationPlugin::SimulationPlugin(SDL_GUI::CommandLine *command_line)
    : SDL_GUI::PluginBase("Simulation Plugin", command_line) {
    this->_command_line->register_flag("nosim", "n", "no_simulation");
    this->_command_line->register_flag("cbs", "c", "simulate_cbs");
    this->_command_line->register_flag("grub", "g", "simulate_grub");
    this->_command_line->register_flag("deadline", "d", "simulate_deadline");
    this->_command_line->register_option("output", "o", "output");
    this->_command_line->register_positional("input");
}


template<>
AtlasSimulationModel *
SimulationPlugin::parse_file<AtlasSimulationModel>(std::istream *input) const {
    return this->parse_file<AtlasSimulationModel, AtlasParser>(input);
}

template<>
CbsSimulationModel *
SimulationPlugin::parse_file<CbsSimulationModel>(std::istream *input) const {
    return this->parse_file<CbsSimulationModel, CbsParser>(input);
}

template<>
GrubSimulationModel *
SimulationPlugin::parse_file<GrubSimulationModel>(std::istream *input) const {
    return this->parse_file<GrubSimulationModel, GrubParser>(input);
}

template<>
DeadlineSimulationModel *
SimulationPlugin::parse_file<DeadlineSimulationModel>(std::istream *input) const {
    return nullptr; //return this->parse_file<DeadlineSimulationModel, CbsParser<DeadlineSimulationModel>>(input);
}
