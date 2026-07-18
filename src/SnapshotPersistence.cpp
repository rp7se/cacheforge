#include "cacheforge/SnapshotPersistence.h"

#include "cacheforge/KVStore.h"

#include <charconv>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <system_error>
#include <utility>

namespace {

constexpr const char* kSnapshotHeader = "CACHEFORGE_SNAPSHOT_V1";

bool parse_size(const std::string& text, std::size_t& value) {
    if (text.empty()) {
        return false;
    }

    const auto result =
        std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool parse_entry_sizes(const std::string& line,
                       std::size_t& key_size,
                       std::size_t& value_size) {
    const std::size_t separator = line.find(' ');
    if (separator == std::string::npos ||
        line.find(' ', separator + 1) != std::string::npos) {
        return false;
    }

    return parse_size(line.substr(0, separator), key_size) &&
           parse_size(line.substr(separator + 1), value_size);
}

bool read_exact(std::ifstream& input, std::string& data, std::size_t size) {
    if (size > static_cast<std::size_t>(
                   (std::numeric_limits<std::streamsize>::max)())) {
        return false;
    }

    const std::streampos current_position = input.tellg();
    if (current_position < 0) {
        return false;
    }

    input.seekg(0, std::ios::end);
    const std::streampos end_position = input.tellg();
    input.seekg(current_position);
    if (end_position < current_position ||
        size > static_cast<std::size_t>(end_position - current_position)) {
        return false;
    }

    data.resize(size);
    if (size == 0) {
        return true;
    }

    input.read(data.data(), static_cast<std::streamsize>(size));
    return input.gcount() == static_cast<std::streamsize>(size);
}

}  // namespace

namespace cacheforge {

SnapshotPersistence::SnapshotPersistence(std::string file_path)
    : file_path_(std::move(file_path)) {}

bool SnapshotPersistence::save(const KVStore& store,
                               std::string& error_message) const try {
    error_message.clear();
    const KVStore::PersistentEntries entries = store.getPersistentEntries();

    std::ofstream output(file_path_, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        error_message = "cannot open snapshot file for writing: " + file_path_;
        return false;
    }

    output << kSnapshotHeader << '\n' << entries.size() << '\n';
    for (const auto& [key, value] : entries) {
        output << key.size() << ' ' << value.size() << '\n';
        output.write(key.data(), static_cast<std::streamsize>(key.size()));
        output.write(value.data(), static_cast<std::streamsize>(value.size()));
        output.put('\n');
    }

    output.close();
    if (!output) {
        error_message = "failed while writing snapshot file: " + file_path_;
        return false;
    }

    return true;
} catch (const std::exception& error) {
    error_message =
        "unexpected snapshot save error for " + file_path_ + ": " + error.what();
    return false;
}

bool SnapshotPersistence::load(KVStore& store,
                               std::string& error_message) const try {
    error_message.clear();

    std::error_code exists_error;
    const bool snapshot_exists = std::filesystem::exists(file_path_, exists_error);
    if (exists_error) {
        error_message = "cannot check snapshot file: " + file_path_;
        return false;
    }
    if (!snapshot_exists) {
        return true;
    }

    std::ifstream input(file_path_, std::ios::binary);
    if (!input.is_open()) {
        error_message = "cannot open snapshot file for reading: " + file_path_;
        return false;
    }

    std::string line;
    if (!std::getline(input, line) || line != kSnapshotHeader) {
        error_message = "invalid snapshot header: " + file_path_;
        return false;
    }

    std::size_t entry_count = 0;
    if (!std::getline(input, line) || !parse_size(line, entry_count)) {
        error_message = "invalid snapshot entry count: " + file_path_;
        return false;
    }

    KVStore::PersistentEntries entries;
    for (std::size_t index = 0; index < entry_count; ++index) {
        std::size_t key_size = 0;
        std::size_t value_size = 0;
        if (!std::getline(input, line) ||
            !parse_entry_sizes(line, key_size, value_size)) {
            error_message = "invalid snapshot entry sizes: " + file_path_;
            return false;
        }

        std::string key;
        std::string value;
        if (!read_exact(input, key, key_size) ||
            !read_exact(input, value, value_size) || input.get() != '\n') {
            error_message = "truncated or invalid snapshot entry: " + file_path_;
            return false;
        }

        entries.emplace_back(std::move(key), std::move(value));
    }

    if (input.peek() != std::char_traits<char>::eof()) {
        error_message = "unexpected trailing snapshot data: " + file_path_;
        return false;
    }

    store.restorePersistentEntries(entries);
    return true;
} catch (const std::exception& error) {
    error_message =
        "unexpected snapshot load error for " + file_path_ + ": " + error.what();
    return false;
}

const std::string& SnapshotPersistence::filePath() const {
    return file_path_;
}

}  // namespace cacheforge
