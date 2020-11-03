#pragma once

#include <vector>
#include <string>


/**
 * An Object that holds one or more attributes to be identified
 */
class Attributable {
    std::vector<std::string> attributes; /**< List of attributes */

public:
    /**
     * add an attribute to the object
     * @param attribute
     *   attribute to add
     */
    void add_attribute(std::string attribute);

    /**
     * check if the Object holds a given attribute
     *
     * @param attribute
     *   attribute to check for
     * @returns
     *   True if attribute is in the list of given attributes for this object. False otherwise.
     */
    bool has_attribute(std::string attribute);
};
