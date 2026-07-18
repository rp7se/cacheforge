#pragma once

#include <optional>
#include <string>
#include <vector>

namespace cacheforge {

struct ParsedCommand {
    std::string command;
    std::vector<std::string> arguments;
};

class CommandParser {
public:
    [[nodiscard]] std::optional<ParsedCommand> parse(const std::string& input) const;
};

}  // namespace cacheforge
