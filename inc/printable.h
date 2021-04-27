#pragma once

#include <string>

class Printable {
  public:
    virtual std::string to_string() const = 0;
};
