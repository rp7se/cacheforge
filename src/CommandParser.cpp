#include "cacheforge/CommandParser.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace cacheforge {

std::optional<ParsedCommand> CommandParser::parse(const std::string& input) const {
    std::istringstream input_stream(input);
    ParsedCommand result;

    if (!(input_stream >> result.command)) {
        return std::nullopt;
    }

    std::transform(result.command.begin(), result.command.end(), result.command.begin(),
                   [](unsigned char character) {
                       return static_cast<char>(std::toupper(character));
                   });

    std::string argument;
    while (input_stream >> argument) {
        result.arguments.push_back(argument);
    }

    return result;
}

}  // namespace cacheforge
