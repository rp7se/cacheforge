#include "cacheforge/CommandParser.h"
#include "cacheforge/CommandProcessor.h"
#include "cacheforge/KVStore.h"

#include <iostream>
#include <string>

namespace {

void expect(bool condition, const std::string& description, int& failure_count) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
        return;
    }

    std::cerr << "[FAIL] " << description << '\n';
    ++failure_count;
}

}  // namespace

int main() {
    cacheforge::KVStore store;
    cacheforge::CommandProcessor processor(store);
    int failure_count = 0;

    expect(processor.execute({"SET", {"name", "Tracy"}}) == "OK",
           "1. SET adds a new key", failure_count);
    expect(processor.execute({"GET", {"name"}}) == "Tracy",
           "2. GET returns an existing value", failure_count);
    expect(processor.execute({"GET", {"missing"}}) == "(nil)",
           "3. GET returns (nil) for a missing key", failure_count);

    const bool overwrite_succeeded =
        processor.execute({"SET", {"name", "Bob"}}) == "OK" &&
        processor.execute({"GET", {"name"}}) == "Bob";
    expect(overwrite_succeeded, "4. SET overwrites an existing value", failure_count);

    expect(processor.execute({"DEL", {"name"}}) == "1",
           "5. DEL returns 1 for an existing key", failure_count);
    expect(processor.execute({"DEL", {"name"}}) == "0",
           "6. DEL returns 0 for a missing key", failure_count);

    const bool existing_key_succeeded =
        processor.execute({"SET", {"existing", "value"}}) == "OK" &&
        processor.execute({"EXISTS", {"existing"}}) == "1";
    expect(existing_key_succeeded,
           "7. EXISTS returns 1 for an existing key", failure_count);
    expect(processor.execute({"EXISTS", {"missing"}}) == "0",
           "8. EXISTS returns 0 for a missing key", failure_count);

    expect(processor.execute({"SET", {"key"}}) == "ERROR wrong number of arguments",
           "9. SET rejects too few arguments", failure_count);
    expect(processor.execute({"SET", {"key", "value", "extra"}}) ==
               "ERROR wrong number of arguments",
           "10. SET rejects too many arguments", failure_count);
    expect(processor.execute({"GET", {}}) == "ERROR wrong number of arguments",
           "11. GET rejects too few arguments", failure_count);
    expect(processor.execute({"GET", {"key", "extra"}}) ==
               "ERROR wrong number of arguments",
           "12. GET rejects too many arguments", failure_count);
    expect(processor.execute({"DEL", {}}) == "ERROR wrong number of arguments",
           "13. DEL rejects an invalid argument count", failure_count);
    expect(processor.execute({"EXISTS", {"key", "extra"}}) ==
               "ERROR wrong number of arguments",
           "14. EXISTS rejects an invalid argument count", failure_count);
    expect(processor.execute({"HELLO", {"world"}}) == "ERROR unknown command",
           "15. Unknown command returns an error", failure_count);

    const bool empty_value_succeeded =
        processor.execute({"SET", {"empty", ""}}) == "OK" &&
        processor.execute({"GET", {"empty"}}).empty();
    expect(empty_value_succeeded, "16. Empty string value remains distinct from missing",
           failure_count);

    const cacheforge::CommandParser parser;
    const auto parsed_set = parser.parse("SET integrated Tracy");
    const auto parsed_get = parser.parse("GET integrated");
    const bool integration_succeeded =
        parsed_set.has_value() && parsed_get.has_value() &&
        processor.execute(*parsed_set) == "OK" && processor.execute(*parsed_get) == "Tracy";
    expect(integration_succeeded, "17. Parser, Processor, and KVStore integration",
           failure_count);

    if (failure_count != 0) {
        std::cerr << failure_count << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All 17 CommandProcessor tests passed.\n";
    return 0;
}
