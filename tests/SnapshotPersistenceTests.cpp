#include "cacheforge/KVStore.h"
#include "cacheforge/SnapshotPersistence.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
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

class TemporarySnapshotFile {
public:
    TemporarySnapshotFile()
        : path_(std::filesystem::temp_directory_path() /
                ("cacheforge_snapshot_tests_" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()) +
                 ".snapshot")) {}

    ~TemporarySnapshotFile() {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    TemporarySnapshotFile(const TemporarySnapshotFile&) = delete;
    TemporarySnapshotFile& operator=(const TemporarySnapshotFile&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const {
        return path_;
    }

    void remove() const {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

private:
    std::filesystem::path path_;
};

}  // namespace

int main() {
    using namespace std::chrono_literals;

    int failure_count = 0;
    TemporarySnapshotFile temporary_file;
    const cacheforge::SnapshotPersistence persistence(temporary_file.path().string());
    std::string error_message;

    cacheforge::KVStore source;
    source.set("alpha", "one");
    source.set("beta", "two");
    source.setWithTtl("session", "temporary", 60s);
    source.setWithTtl("converted", "temporary", 60s);
    source.set("converted", "permanent");
    source.set("converted-to-ttl", "ordinary");
    source.setWithTtl("converted-to-ttl", "temporary", 60s);
    const std::string special_key = "key=with\nnewline";
    const std::string special_value = "value=with\nnewline";
    source.set(special_key, special_value);

    expect(persistence.save(source, error_message) && error_message.empty() &&
               std::filesystem::exists(temporary_file.path()),
           "1. Save writes ordinary keys to a snapshot file", failure_count);

    cacheforge::KVStore restored;
    expect(persistence.load(restored, error_message) && error_message.empty() &&
               restored.exists("alpha"),
           "2. Load restores an ordinary key", failure_count);
    expect(restored.exists("alpha") && restored.exists("beta"),
           "3. Load restores multiple keys", failure_count);
    expect(restored.get("alpha") == std::optional<std::string>("one") &&
               restored.get("beta") == std::optional<std::string>("two"),
           "4. Restored values match their saved values", failure_count);

    const auto persistent_entries = source.getPersistentEntries();
    bool ttl_entry_was_copied = false;
    for (const auto& [key, value] : persistent_entries) {
        static_cast<void>(value);
        if (key == "session") {
            ttl_entry_was_copied = true;
        }
    }
    expect(!ttl_entry_was_copied,
           "5. KVStore snapshot view excludes TTL keys", failure_count);
    expect(!restored.exists("session"),
           "6. A TTL key is not restored after restart simulation", failure_count);
    expect(restored.get("converted") ==
               std::optional<std::string>("permanent"),
           "7. A TTL key overwritten by SET becomes persistent", failure_count);
    expect(!restored.exists("converted-to-ttl"),
           "8. An ordinary key overwritten by SETEX is not persistent",
           failure_count);

    temporary_file.remove();
    cacheforge::KVStore first_run_store;
    expect(persistence.load(first_run_store, error_message) &&
               error_message.empty() && !first_run_store.exists("alpha"),
           "9. A missing snapshot file is treated as the first run", failure_count);

    cacheforge::KVStore empty_source;
    expect(persistence.save(empty_source, error_message),
           "10. An empty KVStore can be saved", failure_count);
    cacheforge::KVStore empty_restored;
    expect(persistence.load(empty_restored, error_message) &&
               empty_restored.getPersistentEntries().empty(),
           "11. An empty snapshot can be loaded", failure_count);

    {
        std::ofstream corrupted(temporary_file.path(),
                                std::ios::binary | std::ios::trunc);
        corrupted << "CACHEFORGE_SNAPSHOT_V1\n1\n999999999 1\nx\n";
    }
    cacheforge::KVStore corruption_target;
    corruption_target.set("existing", "kept");
    expect(!persistence.load(corruption_target, error_message) &&
               !error_message.empty() &&
               corruption_target.get("existing") ==
                   std::optional<std::string>("kept"),
           "12. Corrupted snapshots return an error without partial restore",
           failure_count);

    expect(persistence.save(source, error_message),
           "13. A valid snapshot can replace a corrupted file", failure_count);
    cacheforge::KVStore special_restored;
    expect(persistence.load(special_restored, error_message) &&
               special_restored.get(special_key) ==
                   std::optional<std::string>(special_value),
           "14. Length prefixes preserve special characters", failure_count);

    const cacheforge::SnapshotPersistence invalid_destination(
        std::filesystem::temp_directory_path().string());
    expect(!invalid_destination.save(source, error_message) &&
               !error_message.empty(),
           "15. Save reports a clear file-open error", failure_count);

    std::filesystem::path cleanup_path;
    bool cleanup_file_was_created = false;
    {
        TemporarySnapshotFile cleanup_file;
        cleanup_path = cleanup_file.path();
        const cacheforge::SnapshotPersistence cleanup_persistence(
            cleanup_path.string());
        cacheforge::KVStore cleanup_store;
        cleanup_store.set("cleanup", "verified");
        cleanup_file_was_created =
            cleanup_persistence.save(cleanup_store, error_message) &&
            std::filesystem::exists(cleanup_path);
    }
    expect(cleanup_file_was_created && !std::filesystem::exists(cleanup_path),
           "16. Temporary snapshot files are cleaned up after tests",
           failure_count);

    if (failure_count != 0) {
        std::cerr << failure_count << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All 16 Snapshot Persistence tests passed.\n";
    return 0;
}
