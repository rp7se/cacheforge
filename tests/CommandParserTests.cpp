#include "cacheforge/CommandParser.h"

#include <iostream>
#include <optional>
#include <string>
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

bool matches(const std::optional<cacheforge::ParsedCommand>& parsed,
             const std::string& command,
             const std::vector<std::string>& arguments) {
    return parsed.has_value() && parsed->command == command &&
           parsed->arguments == arguments;
}

}  // namespace

int main() {
    const cacheforge::CommandParser parser;
    int failure_count = 0;

    expect(matches(parser.parse("SET name Tracy"), "SET", {"name", "Tracy"}),
           "1. Parse SET command", failure_count);
    expect(matches(parser.parse("GET name"), "GET", {"name"}),
           "2. Parse GET command", failure_count);
    expect(matches(parser.parse("DEL name"), "DEL", {"name"}),
           "3. Parse DEL command", failure_count);
    expect(matches(parser.parse("EXISTS name"), "EXISTS", {"name"}),
           "4. Parse EXISTS command", failure_count);
    expect(matches(parser.parse("set Name Tracy"), "SET", {"Name", "Tracy"}),
           "5. Normalize lowercase command and preserve arguments", failure_count);
    expect(matches(parser.parse("gEt name"), "GET", {"name"}),
           "6. Normalize mixed-case command", failure_count);
    expect(matches(parser.parse("   GET name   "), "GET", {"name"}),
           "7. Ignore leading and trailing whitespace", failure_count);
    expect(matches(parser.parse("SET    name    Tracy"), "SET", {"name", "Tracy"}),
           "8. Ignore repeated whitespace", failure_count);
    expect(!parser.parse("").has_value(), "9. Reject empty input", failure_count);
    expect(!parser.parse("     ").has_value(), "10. Reject whitespace-only input",
           failure_count);
    expect(matches(parser.parse("HELLO world"), "HELLO", {"world"}),
           "11. Parse unknown command", failure_count);
    expect(matches(parser.parse("GET"), "GET", {}),
           "12. Parse command without arguments", failure_count);

    if (failure_count != 0) {
        std::cerr << failure_count << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All 12 CommandParser tests passed.\n";
    return 0;
}
