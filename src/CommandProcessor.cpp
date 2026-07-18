#include "cacheforge/CommandProcessor.h"

#include <charconv>
#include <chrono>
#include <system_error>

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

    if (parsed_command.command == "SETEX") {
        if (parsed_command.arguments.size() != 3) {
            return "ERROR wrong number of arguments";
        }

        const std::string& expiration_text = parsed_command.arguments[1];
        int expiration_seconds = 0;
        const auto parse_result =
            std::from_chars(expiration_text.data(),
                            expiration_text.data() + expiration_text.size(),
                            expiration_seconds);
        if (parse_result.ec != std::errc{} ||
            parse_result.ptr != expiration_text.data() + expiration_text.size() ||
            expiration_seconds <= 0) {
            return "ERROR invalid expiration";
        }

        store_.setWithTtl(parsed_command.arguments[0], parsed_command.arguments[2],
                          std::chrono::seconds(expiration_seconds));
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
