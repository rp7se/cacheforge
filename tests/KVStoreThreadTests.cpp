#include "cacheforge/KVStore.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kThreadCount = 8;
constexpr int kKeysPerThread = 100;

std::string make_key(int thread_index, int key_index) {
    return "thread-" + std::to_string(thread_index) + "-key-" +
           std::to_string(key_index);
}

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
    std::vector<std::thread> threads;

    for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
        threads.emplace_back([&store, thread_index]() {
            for (int key_index = 0; key_index < kKeysPerThread; ++key_index) {
                store.set(make_key(thread_index, key_index), std::to_string(key_index));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    bool all_values_present = true;
    for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
        for (int key_index = 0; key_index < kKeysPerThread; ++key_index) {
            const auto value = store.get(make_key(thread_index, key_index));
            if (!value.has_value() || *value != std::to_string(key_index)) {
                all_values_present = false;
            }
        }
    }
    expect(all_values_present, "1. Concurrent SET and subsequent GET", failure_count);

    threads.clear();
    std::vector<int> exists_results(kThreadCount, 0);
    for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
        threads.emplace_back([&store, &exists_results, thread_index]() {
            bool all_exist = true;
            for (int key_index = 0; key_index < kKeysPerThread; ++key_index) {
                if (!store.exists(make_key(thread_index, key_index))) {
                    all_exist = false;
                }
            }
            exists_results[thread_index] = all_exist ? 1 : 0;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    bool all_exists_checks_passed = true;
    for (const int result : exists_results) {
        if (result != 1) {
            all_exists_checks_passed = false;
        }
    }
    expect(all_exists_checks_passed, "2. Concurrent EXISTS", failure_count);

    threads.clear();
    std::vector<int> delete_counts(kThreadCount, 0);
    for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
        threads.emplace_back([&store, &delete_counts, thread_index]() {
            int deleted = 0;
            for (int key_index = 0; key_index < kKeysPerThread; key_index += 2) {
                if (store.del(make_key(thread_index, key_index))) {
                    ++deleted;
                }
            }
            delete_counts[thread_index] = deleted;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    bool delete_counts_correct = true;
    for (const int count : delete_counts) {
        if (count != kKeysPerThread / 2) {
            delete_counts_correct = false;
        }
    }
    expect(delete_counts_correct, "3. Concurrent DEL reports expected counts",
           failure_count);

    bool final_state_correct = true;
    for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
        for (int key_index = 0; key_index < kKeysPerThread; ++key_index) {
            const bool should_exist = key_index % 2 != 0;
            if (store.exists(make_key(thread_index, key_index)) != should_exist) {
                final_state_correct = false;
            }
        }
    }
    expect(final_state_correct, "4. Final shared data state", failure_count);

    if (failure_count != 0) {
        std::cerr << failure_count << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All KVStore thread-safety tests passed.\n";
    return 0;
}
