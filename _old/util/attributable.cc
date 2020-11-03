#include <util/attributable.h>

#include <algorithm>

void Attributable::add_attribute(std::string attribute) {
    this->attributes.push_back(attribute);
}

bool Attributable::has_attribute(std::string attribute) {
    return (std::find(this->attributes.begin(), this->attributes.end(), attribute) != this->attributes.end());
}
