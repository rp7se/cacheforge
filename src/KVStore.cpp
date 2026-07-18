#include "cacheforge/KVStore.h"

namespace cacheforge {

void KVStore::set(const std::string& key, const std::string& value) {
    data_[key] = value;
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    const auto entry = data_.find(key);
    if (entry == data_.end()) {
        return std::nullopt;
    }

    return entry->second;
}

bool KVStore::del(const std::string& key) {
    return data_.erase(key) > 0;
}

bool KVStore::exists(const std::string& key) const {
    return data_.find(key) != data_.end();
}

}  // namespace cacheforge
