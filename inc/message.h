#pragma once

#include <set>
#include <string>

/** A Message the Simulation has decided to be displayed at a certain timestamp. */
class Message {
public:
    int _timestamp;         /**< timestamp the message occured */
    std::string _message;   /**< message string */
    std::set<unsigned> _jobs;    /**< jobs this message relates to */

    /**
     * Constructor
     * @param timestamp timestamp the message occured
     * @param message message string
     */
    Message(int timestamp, std::string message, std::set<unsigned> jobs = std::set<unsigned>())
        : _timestamp(timestamp), _message(message), _jobs(jobs) {};
};
