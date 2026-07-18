#include "cacheforge/CommandProcessor.h"

namespace cacheforge {

CommandProcessor::CommandProcessor(KVStore& store) : store_(store) {}

std::string CommandProcessor::execute(const ParsedCommand& parsed_command) {
    if (parsed_command.command == "SET") {
        if (parsed_command.arguments.size() != 2) {
            return "ERROR wrong number of arguments";
        }

        store_.set(parsed_command.arguments[0], parsed_command.arguments[1]);
        return "OK";
    }

    if (parsed_command.command == "GET") {
        if (parsed_command.arguments.size() != 1) {
            return "ERROR wrong number of arguments";
        }

        const auto value = store_.get(parsed_command.arguments[0]);
        return value.has_value() ? *value : "(nil)";
    }

    if (parsed_command.command == "DEL") {
        if (parsed_command.arguments.size() != 1) {
            return "ERROR wrong number of arguments";
        }

        return store_.del(parsed_command.arguments[0]) ? "1" : "0";
    }

    if (parsed_command.command == "EXISTS") {
        if (parsed_command.arguments.size() != 1) {
            return "ERROR wrong number of arguments";
        }

        return store_.exists(parsed_command.arguments[0]) ? "1" : "0";
    }

    return "ERROR unknown command";
}

}  // namespace cacheforge
