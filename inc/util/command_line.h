#pragma once

#include <map>
#include <string>
#include <vector>

struct CommandLineOption {
    std::string _name;
    std::string _key;
    std::string _shortcut;
};

class CommandLine {
    std::map<std::string, std::string> _arguments;
    std::vector<CommandLineOption> _options;

    void parse_argument(std::string key, std::string value, bool is_shortcut);
public:
    void register_option(std::string key, std::string shortcut = "", std::string name = "");
    void parse(int argc, char *argv[]);
    std::string get_argument(std::string key) const;
};
