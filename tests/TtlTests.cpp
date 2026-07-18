#include "cacheforge/CommandProcessor.h"
#include "cacheforge/KVStore.h"

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

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
    using namespace std::chrono_literals;

    cacheforge::KVStore store;
    cacheforge::CommandProcessor processor(store);
    int failure_count = 0;

    expect(processor.execute({"SETEX", {"session", "1", "abc"}}) == "OK" &&
               processor.execute({"GET", {"session"}}) == "abc",
           "1. SETEX is immediately readable", failure_count);

    store.setWithTtl("expired-get", "value", 1s);
    store.setWithTtl("expired-exists", "value", 1s);
    store.setWithTtl("expired-del", "value", 1s);
    store.set("permanent", "value");
    store.setWithTtl("clear-ttl", "temporary", 1s);
    store.set("clear-ttl", "permanent");
    store.set("ttl-overwrite", "permanent");
    store.setWithTtl("ttl-overwrite", "temporary", 1s);

    std::this_thread::sleep_for(1200ms);

    expect(!store.get("expired-get").has_value() &&
               processor.execute({"GET", {"session"}}) == "(nil)",
           "2. GET treats expired keys as missing", failure_count);
    expect(!store.exists("expired-exists"),
           "3. EXISTS treats expired keys as missing", failure_count);
    expect(!store.del("expired-del"), "4. DEL treats expired keys as missing",
           failure_count);
    expect(store.get("permanent") == std::optional<std::string>("value"),
           "5. Ordinary SET does not expire", failure_count);
    expect(store.get("clear-ttl") == std::optional<std::string>("permanent"),
           "6. Ordinary SET clears an existing TTL", failure_count);
    expect(!store.get("ttl-overwrite").has_value(),
           "7. SETEX overwrites an ordinary key", failure_count);

    expect(processor.execute({"SETEX", {"key", "1"}}) ==
               "ERROR wrong number of arguments" &&
               processor.execute({"SETEX", {"key", "1", "value", "extra"}}) ==
                   "ERROR wrong number of arguments",
           "8. SETEX requires exactly three arguments", failure_count);
    expect(processor.execute({"SETEX", {"key", "invalid", "value"}}) ==
               "ERROR invalid expiration" &&
               processor.execute({"SETEX", {"key", "1x", "value"}}) ==
                   "ERROR invalid expiration",
           "9. SETEX rejects invalid expiration text", failure_count);
    expect(processor.execute({"SETEX", {"key", "0", "value"}}) ==
               "ERROR invalid expiration" &&
               processor.execute({"SETEX", {"key", "-1", "value"}}) ==
                   "ERROR invalid expiration",
           "10. SETEX rejects zero and negative expiration", failure_count);

    store.setWithTtl("remove-expired", "value", 0s);
    expect(store.removeExpired() == 1 && !store.exists("remove-expired"),
           "11. removeExpired removes expired entries", failure_count);

    store.setWithTtl("periodic-cleanup", "value", 1s);
    std::this_thread::sleep_for(2200ms);
    expect(store.removeExpired() == 0 && !store.exists("periodic-cleanup"),
           "12. Background thread periodically removes expired entries",
           failure_count);

    constexpr int kThreadCount = 8;
    std::vector<std::thread> threads;
    std::vector<int> thread_results(kThreadCount, 0);
    for (int index = 0; index < kThreadCount; ++index) {
        threads.emplace_back([&store, &thread_results, index]() {
            const std::string key = "ttl-thread-" + std::to_string(index);
            store.setWithTtl(key, "value", std::chrono::seconds(5));
            const bool valid = store.get(key) == std::optional<std::string>("value") &&
                               store.exists(key) && store.del(key);
            thread_results[index] = valid ? 1 : 0;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    bool all_thread_operations_passed = true;
    for (const int result : thread_results) {
        if (result != 1) {
            all_thread_operations_passed = false;
        }
    }
    expect(all_thread_operations_passed,
           "13. Concurrent TTL operations preserve thread safety", failure_count);

    if (failure_count != 0) {
        std::cerr << failure_count << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All 13 TTL tests passed.\n";
    return 0;
}
