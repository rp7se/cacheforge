#pragma once

#include <string>

namespace cacheforge {

class KVStore;

class SnapshotPersistence {
public:
    explicit SnapshotPersistence(std::string file_path = "cacheforge.snapshot");

    [[nodiscard]] bool save(const KVStore& store, std::string& error_message) const;
    [[nodiscard]] bool load(KVStore& store, std::string& error_message) const;

    [[nodiscard]] const std::string& filePath() const;

private:
    std::string file_path_;
};

}  // namespace cacheforge
