#pragma once

#include "cacheforge/CommandParser.h"
#include "cacheforge/KVStore.h"

#include <string>

namespace cacheforge {

class CommandProcessor {
public:
    explicit CommandProcessor(KVStore& store);

    [[nodiscard]] std::string execute(const ParsedCommand& parsed_command);

private:
    KVStore& store_;
};

}  // namespace cacheforge
