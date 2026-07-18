#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace cacheforge {

class KVStore {
public:
    void set(const std::string& key, const std::string& value);

    [[nodiscard]] std::optional<std::string> get(const std::string& key) const;
    [[nodiscard]] bool del(const std::string& key);
    [[nodiscard]] bool exists(const std::string& key) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> data_;
};

}  // namespace cacheforge
