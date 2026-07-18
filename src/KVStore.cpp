#include "cacheforge/KVStore.h"

namespace cacheforge {

KVStore::KVStore() : cleanup_thread_(&KVStore::cleanupLoop, this) {}

KVStore::~KVStore() {
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        stop_cleanup_ = true;
    }

    cleanup_condition_.notify_one();
    cleanup_thread_.join();
}

void KVStore::set(const std::string& key, const std::string& value) {
    const std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = Entry{value, std::nullopt};
}

void KVStore::setWithTtl(const std::string& key,
                         const std::string& value,
                         std::chrono::seconds ttl) {
    const std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = Entry{value, Clock::now() + ttl};
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    const std::lock_guard<std::mutex> lock(mutex_);
    const auto entry = data_.find(key);
    if (entry == data_.end()) {
        return std::nullopt;
    }

    if (isExpired(entry->second, Clock::now())) {
        data_.erase(entry);
        return std::nullopt;
    }

    return entry->second.value;
}

bool KVStore::del(const std::string& key) {
    const std::lock_guard<std::mutex> lock(mutex_);
    const auto entry = data_.find(key);
    if (entry == data_.end()) {
        return false;
    }

    if (isExpired(entry->second, Clock::now())) {
        data_.erase(entry);
        return false;
    }

    data_.erase(entry);
    return true;
}

bool KVStore::exists(const std::string& key) const {
    const std::lock_guard<std::mutex> lock(mutex_);
    const auto entry = data_.find(key);
    if (entry == data_.end()) {
        return false;
    }

    if (isExpired(entry->second, Clock::now())) {
        data_.erase(entry);
        return false;
    }

    return true;
}

std::size_t KVStore::removeExpired() {
    const std::lock_guard<std::mutex> lock(mutex_);
    return removeExpiredLocked(Clock::now());
}

bool KVStore::isExpired(const Entry& entry, Clock::time_point now) {
    return entry.expire_at.has_value() && *entry.expire_at <= now;
}

std::size_t KVStore::removeExpiredLocked(Clock::time_point now) {
    std::size_t removed_count = 0;

    for (auto entry = data_.begin(); entry != data_.end();) {
        if (isExpired(entry->second, now)) {
            entry = data_.erase(entry);
            ++removed_count;
        } else {
            ++entry;
        }
    }

    return removed_count;
}

void KVStore::cleanupLoop() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (!stop_cleanup_) {
        const bool stopping = cleanup_condition_.wait_for(
            lock, std::chrono::seconds(1), [this]() { return stop_cleanup_; });
        if (stopping) {
            break;
        }

        removeExpiredLocked(Clock::now());
    }
}

}  // namespace cacheforge
