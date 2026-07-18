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
    int failure_count = 0;

    store.set("name", "Alice");
    expect(store.exists("name"), "1. SET adds a new key", failure_count);

    const auto existing_value = store.get("name");
    expect(existing_value.has_value() && *existing_value == "Alice",
           "2. GET returns an existing value", failure_count);

    expect(!store.get("missing").has_value(), "3. GET reports a missing key",
           failure_count);

    store.set("name", "Bob");
    const auto updated_value = store.get("name");
    expect(updated_value.has_value() && *updated_value == "Bob",
           "4. SET overwrites an existing value", failure_count);

    store.set("delete-me", "value");
    expect(store.del("delete-me") && !store.exists("delete-me"),
           "5. DEL removes an existing key", failure_count);

    expect(!store.del("missing"), "6. DEL reports a missing key", failure_count);
    expect(store.exists("name"), "7. EXISTS finds an existing key", failure_count);
    expect(!store.exists("missing"), "8. EXISTS reports a missing key", failure_count);

    store.set("empty", "");
    const auto empty_value = store.get("empty");
    expect(empty_value.has_value() && empty_value->empty(),
           "9. Empty string is distinct from a missing key", failure_count);

    if (failure_count != 0) {
        std::cerr << failure_count << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All 9 KVStore tests passed.\n";
    return 0;
}
