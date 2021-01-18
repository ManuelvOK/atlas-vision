#include <util/command_line.h>

#include <iostream>

void CommandLine::parse_argument(std::string key, std::string value, bool is_shortcut) {
    for (CommandLineOption &option: this->_options) {
        if ((is_shortcut && option._shortcut == key)
            || (not is_shortcut && option._key == key)) {
            this->_arguments.emplace(option._name, value);
        }
    }
}

void CommandLine::register_option(std::string key, std::string shortcut, std::string name) {
    if (name.empty()) {
        name = key;
    }
    this->_options.emplace_back(CommandLineOption{name, key, shortcut});
}

void CommandLine::parse(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        /* discard arguments without key for now */
        /* TODO: It should be possible to add positional arguments */
        if (arg[0] != '-') {
            std::cerr << "Could not process command line argument: \"" << arg
                      << "\". Positional arguments are not supported yet" << std::endl;
            continue;
        }

        /* check if given key is a shortcut */
        arg = arg.substr(1);
        bool is_shortcut = (arg[0] != '-');
        if (not is_shortcut) {
            arg = arg.substr(1);
        }

        /* check if a value is given */
        /* TODO: It should be possible to add options without value (like --help) */
        std::string next_arg = argv[i + 1];
        if (next_arg[0] == '-') {
            std::cerr << "Command line argument \"" << arg << "\" has no value." << std::endl;
            continue;
        }

        this->parse_argument(arg, next_arg, is_shortcut);
        ++i;
    }
}

std::string CommandLine::get_argument(std::string key) const {
    if (this->_arguments.contains(key)) {
        return this->_arguments.at(key);
    }
    return "";
}
