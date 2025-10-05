#include "cliente.hpp"
#include "tslog.hpp"
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

ChatClient::ChatClient(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

ChatClient::~ChatClient() { disconnect(); }

bool ChatClient::connect_to_server() {
    if (running_.load()) return true;
    sock_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd_ < 0) {
        tslog::error("client: socket() failed");
        return false;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));
    if (::inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) {
        tslog::error("client: inet_pton failed (use IPv4 ex: 127.0.0.1)");
        ::close(sock_fd_);
        sock_fd_ = -1;
        return false;
    }
    if (::connect(sock_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        tslog::error("client: connect() failed");
        ::close(sock_fd_);
        sock_fd_ = -1;
        return false;
    }
    running_.store(true);
    tslog::info("client: connected");
    recv_th_ = std::thread(&ChatClient::recv_loop, this);
    return true;
}

void ChatClient::disconnect() {
    if (!running_.exchange(false)) return;
    if (sock_fd_ != -1) {
        ::shutdown(sock_fd_, SHUT_RDWR);
        ::close(sock_fd_);
        sock_fd_ = -1;
    }
    if (recv_th_.joinable()) recv_th_.join();
    tslog::info("client: disconnected");
}

bool ChatClient::send_line(const std::string& line) {
    if (!running_.load() || sock_fd_ == -1) return false;
    std::string buf = line;
    if (buf.empty() || buf.back() != '\n') buf.push_back('\n');
    size_t sent = 0;
    while (sent < buf.size()) {
        ssize_t n = ::send(sock_fd_, buf.data() + sent, buf.size() - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

void ChatClient::recv_loop() {
    std::string acc;
    acc.reserve(1024);
    char c;
    while (running_.load()) {
        ssize_t n = ::recv(sock_fd_, &c, 1, 0);
        if (n == 0) break;            // servidor fechou
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (c == '\n') {
            std::cout << acc << std::endl;
            acc.clear();
        } else {
            acc.push_back(c);
            if (acc.size() > 4096) acc.clear();
        }
    }
}
