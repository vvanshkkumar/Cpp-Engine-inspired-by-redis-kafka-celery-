#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cctype>

struct Command {
    std::string name;
    std::vector<std::string> args;
    bool valid{true};
};

class CommandParser {
public:
    static Command parse(const std::string& raw) {
        Command cmd;
        std::istringstream ss(raw);
        std::string token;
        bool first = true;

        while (ss >> token) {
            if (first) {
                cmd.name = token;

                for (auto& c : cmd.name)
                    c = std::toupper(static_cast<unsigned char>(c));

                first = false;
            } else {
                cmd.args.push_back(token);
            }
        }

        if (cmd.name.empty())
            cmd.valid = false;

        return cmd;
    }
};