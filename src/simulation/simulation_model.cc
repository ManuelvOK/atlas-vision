#include <simulation/simulation_model.h>

BaseSimulationModel::~BaseSimulationModel() {
    for (SimulationAction *action: this->_actions_to_do) {
        delete action;
    }
    for (SimulationAction *action: this->_actions_done) {
        delete action;
    }
    for (Message *message: this->_messages) {
        delete message;
    }
}


void BaseSimulationModel::add_message(int timestamp, std::string text, std::set<unsigned> jobs) {
    Message *message = new Message(timestamp, text, jobs);
    this->_messages.push_back(message);
    std::cerr << timestamp << ": " << text << std::endl;
}

