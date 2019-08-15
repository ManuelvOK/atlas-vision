#pragma once

#include <vector>
#include <string>

class Attributable {
    std::vector<std::string> attributes;

public:
    void add_attribute(std::string attribute);
    bool has_attribute(std::string attribute);
};
