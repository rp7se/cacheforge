#include "cacheforge/KVStore.h"

namespace cacheforge {

void KVStore::set(const std::string& key, const std::string& value) {
    const std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = value;
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    const std::lock_guard<std::mutex> lock(mutex_);
    const auto entry = data_.find(key);
    if (entry == data_.end()) {
        return std::nullopt;
    }

    return entry->second;
}

bool KVStore::del(const std::string& key) {
    const std::lock_guard<std::mutex> lock(mutex_);
    return data_.erase(key) > 0;
}

bool KVStore::exists(const std::string& key) const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return data_.find(key) != data_.end();
}

}  // namespace cacheforge
