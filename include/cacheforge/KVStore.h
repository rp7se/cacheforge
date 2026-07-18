#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cacheforge {

class KVStore {
public:
    using PersistentEntries = std::vector<std::pair<std::string, std::string>>;

    KVStore();
    ~KVStore();

    KVStore(const KVStore&) = delete;
    KVStore& operator=(const KVStore&) = delete;

    void set(const std::string& key, const std::string& value);
    void setWithTtl(const std::string& key,
                    const std::string& value,
                    std::chrono::seconds ttl);

    [[nodiscard]] std::optional<std::string> get(const std::string& key) const;
    [[nodiscard]] bool del(const std::string& key);
    [[nodiscard]] bool exists(const std::string& key) const;
    [[nodiscard]] PersistentEntries getPersistentEntries() const;
    void restorePersistentEntries(const PersistentEntries& entries);
    std::size_t removeExpired();

private:
    using Clock = std::chrono::steady_clock;

    struct Entry {
        std::string value;
        std::optional<Clock::time_point> expire_at;
    };

    [[nodiscard]] static bool isExpired(const Entry& entry, Clock::time_point now);
    std::size_t removeExpiredLocked(Clock::time_point now);
    void cleanupLoop();

    mutable std::mutex mutex_;
    mutable std::unordered_map<std::string, Entry> data_;
    bool stop_cleanup_ = false;
    std::condition_variable cleanup_condition_;
    std::thread cleanup_thread_;
};

}  // namespace cacheforge
