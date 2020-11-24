#pragma once

#include <string>

/** A Message the Simulation has decided to be displayed at a certain timestamp. */
class Message {
public:
    int _timestamp;         /**< timestamp the message occured */
    std::string _message;   /**< message string */

    /**
     * Constructor
     * @param timestamp timestamp the message occured
     * @param message message string
     */
    Message(int timestamp, std::string message)
        : _timestamp(timestamp), _message(message) {};
};
