#pragma once

#include <cstdint>
#include <string>

namespace cacheforge {

class CommandProcessor;

class TcpServer {
public:
    explicit TcpServer(CommandProcessor& processor,
                       std::string address = "127.0.0.1",
                       std::uint16_t port = 6380);

    [[nodiscard]] int run();

private:
    CommandProcessor& processor_;
    std::string address_;
    std::uint16_t port_;
};

}  // namespace cacheforge
