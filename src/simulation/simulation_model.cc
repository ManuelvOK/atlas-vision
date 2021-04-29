#include <simulation/simulation_model.h>


void BaseSimulationModel::add_message(unsigned timestamp, std::string text, std::set<unsigned> jobs) {
    Message *message = new Message(timestamp, text, jobs);
    this->_messages.push_back(message);
    std::cerr << timestamp << ": " << text << std::endl;
}

