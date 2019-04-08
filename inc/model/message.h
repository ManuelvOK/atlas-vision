#pragma once

#include <string>

class Message {
public:
    int timestamp;
    std::string message;
    Message(int timestamp, std::string message)
        : timestamp(timestamp), message(message) {};
};
