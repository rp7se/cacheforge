#include "cacheforge/TcpServer.h"

#include "cacheforge/CommandParser.h"
#include "cacheforge/CommandProcessor.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <array>
#include <iostream>
#include <string>
#include <utility>

namespace {

class WinsockSession {
public:
    WinsockSession() : status_(WSAStartup(MAKEWORD(2, 2), &data_)) {}

    ~WinsockSession() {
        if (status_ == 0) {
            WSACleanup();
        }
    }

    WinsockSession(const WinsockSession&) = delete;
    WinsockSession& operator=(const WinsockSession&) = delete;

    [[nodiscard]] bool initialized() const {
        return status_ == 0;
    }

    [[nodiscard]] int status() const {
        return status_;
    }

private:
    WSADATA data_{};
    int status_;
};

class SocketHandle {
public:
    explicit SocketHandle(SOCKET socket) : socket_(socket) {}

    ~SocketHandle() {
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
        }
    }

    SocketHandle(const SocketHandle&) = delete;
    SocketHandle& operator=(const SocketHandle&) = delete;

    [[nodiscard]] SOCKET get() const {
        return socket_;
    }

    [[nodiscard]] bool valid() const {
        return socket_ != INVALID_SOCKET;
    }

private:
    SOCKET socket_;
};

bool send_all(SOCKET client_socket, const std::string& data) {
    std::size_t sent_bytes = 0;

    while (sent_bytes < data.size()) {
        const int sent = send(client_socket, data.data() + sent_bytes,
                              static_cast<int>(data.size() - sent_bytes), 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "send failed: " << WSAGetLastError() << '\n';
            return false;
        }

        sent_bytes += static_cast<std::size_t>(sent);
    }

    return true;
}

void handle_client(SOCKET client_socket, cacheforge::CommandProcessor& processor) {
    cacheforge::CommandParser parser;
    std::array<char, 4096> chunk{};
    std::string receive_buffer;

    while (true) {
        const int received = recv(client_socket, chunk.data(),
                                  static_cast<int>(chunk.size()), 0);
        if (received == 0) {
            return;
        }

        if (received == SOCKET_ERROR) {
            std::cerr << "recv failed: " << WSAGetLastError() << '\n';
            return;
        }

        receive_buffer.append(chunk.data(), static_cast<std::size_t>(received));

        std::size_t newline_position = 0;
        while ((newline_position = receive_buffer.find('\n')) != std::string::npos) {
            std::string command = receive_buffer.substr(0, newline_position);
            receive_buffer.erase(0, newline_position + 1);

            if (!command.empty() && command.back() == '\r') {
                command.pop_back();
            }

            const auto parsed_command = parser.parse(command);
            std::string response = parsed_command.has_value()
                                       ? processor.execute(*parsed_command)
                                       : "ERROR empty command";
            response.push_back('\n');

            if (!send_all(client_socket, response)) {
                return;
            }
        }
    }
}

}  // namespace

namespace cacheforge {

TcpServer::TcpServer(CommandProcessor& processor, std::string address, std::uint16_t port)
    : processor_(processor), address_(std::move(address)), port_(port) {}

int TcpServer::run() {
    const WinsockSession winsock;
    if (!winsock.initialized()) {
        std::cerr << "WSAStartup failed: " << winsock.status() << '\n';
        return 1;
    }

    const SocketHandle listening_socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!listening_socket.valid()) {
        std::cerr << "socket creation failed: " << WSAGetLastError() << '\n';
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_);

    if (inet_pton(AF_INET, address_.c_str(), &server_address.sin_addr) != 1) {
        std::cerr << "invalid listen address: " << address_ << '\n';
        return 1;
    }

    if (bind(listening_socket.get(), reinterpret_cast<const sockaddr*>(&server_address),
             sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << '\n';
        return 1;
    }

    if (listen(listening_socket.get(), SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << '\n';
        return 1;
    }

    std::cout << "CacheForge Server\n"
              << "Listening on " << address_ << ':' << port_ << '\n';

    while (true) {
        const SocketHandle client_socket(accept(listening_socket.get(), nullptr, nullptr));
        if (!client_socket.valid()) {
            std::cerr << "accept failed: " << WSAGetLastError() << '\n';
            return 1;
        }

        std::cout << "Client connected\n";
        handle_client(client_socket.get(), processor_);
        std::cout << "Client disconnected\n";
    }
}

}  // namespace cacheforge
