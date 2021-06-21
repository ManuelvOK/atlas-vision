#include <util/parser.h>

#include <iostream>
#include <string>
#include <sstream>

#include <atlas/atlas_simulation_model.h>
#include <atlas/cfs_visibility.h>
#include <cbs/cbs_simulation_model.h>
#include <job.h>
#include <message.h>
#include <schedule.h>
#include <simulation/schedule_change.h>

void BaseParser::parse_n_cores(std::stringstream *line) {
    *line >> this->_n_cores;
}

void BaseParser::parse_message(std::stringstream *line) {
    int timestamp;
    *line >> timestamp;
    unsigned pos = line->tellg();
    std::string message = line->str();
    message = message.substr(message.find_first_not_of(" ", pos));
    Message *m = new Message(timestamp, message);
    this->_messages.emplace_back(m);
}


