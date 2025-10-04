#include "server.hpp"
#include "tslog.hpp"

#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>      


namespace {

bool recv_line(int fd, std::string &out) {
    out.clear();
    char c;
    ssize_t n;
    while (true) {
        n = ::recv(fd, &c, 1, 0);
        if (n == 0) return false;
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (c == '\n') return true;
        out.push_back(c);
        if (out.size() > 8192) return false; // proteção simples
    }
}

bool send_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, buf + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

} 

ChatServer::ChatServer(int port) : port_(port) {}
ChatServer::~ChatServer() { stop(); }

bool ChatServer::start() {
    if (running_.load()) return true;

    std::signal(SIGPIPE, SIG_IGN);

    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) { tslog::error("server: socket() failed"); return false; }

    int yes = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port_));

    if (::bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        tslog::error("server: bind() failed");
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    if (::listen(listen_fd_, 16) < 0) {
        tslog::error("server: listen() failed");
        ::close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    running_.store(true);

    {
        std::ostringstream ss; ss << "server: listening on port " << port_;
        tslog::info(ss.str().c_str());
    }

    accept_loop(); // bloqueia aqui
    return true;
}

void ChatServer::stop() {
    if (!running_.exchange(false)) return;

    if (listen_fd_ != -1) {
        ::shutdown(listen_fd_, SHUT_RDWR);
        ::close(listen_fd_);
        listen_fd_ = -1;
    }

    std::lock_guard<std::mutex> lk(mtx_);
    for (int &fd : clients_) {
        if (fd != -1) {
            ::shutdown(fd, SHUT_RDWR);
            ::close(fd);
            fd = -1;
        }
    }
    clients_.clear();

    tslog::info("server: stopped");
}

size_t ChatServer::client_count() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return clients_.size();
}

void ChatServer::accept_loop() {
    tslog::info("server: accept_loop started");
    while (running_.load()) {
        sockaddr_in cli{}; socklen_t clilen = sizeof(cli);
        int cfd = ::accept(listen_fd_, (sockaddr*)&cli, &clilen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (!running_.load()) break;
            tslog::warn("server: accept() failed");
            continue;
        }

        char ip[64]; ::inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        uint16_t cport = ntohs(cli.sin_port);
        { std::ostringstream ss; ss << "server: client connected from " << ip << ":" << cport; tslog::info(ss.str().c_str()); }

        {
            std::lock_guard<std::mutex> lk(mtx_);
            clients_.push_back(cfd);
        }
        std::thread(&ChatServer::client_worker, this, cfd).detach();
    }
    tslog::info("server: accept_loop ended");
}

void ChatServer::client_worker(int client_fd) {
    const std::string hello = "Bem-vindo ao chat!\n";
    (void)send_all(client_fd, hello.c_str(), hello.size());

    std::string line;
    while (running_.load()) {
        if (!recv_line(client_fd, line)) break;

        if (line.size() > 1024) line.resize(1024);
        std::string msg = line + "\n";

        {
    std::ostringstream ss;
    ss << "server: recv line: \"" << line << "\" (" << line.size() << " bytes)";
    tslog::info(ss.str().c_str());
}

        broadcast_line(msg, client_fd);
    }

    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto it = clients_.begin(); it != clients_.end(); ++it) {
            if (*it == client_fd) {
                ::close(client_fd);
                clients_.erase(it);
                break;
            }
        }
    }
    tslog::info("server: client disconnected");
}

void ChatServer::broadcast_line(const std::string& line, int from_fd) {
    std::lock_guard<std::mutex> lk(mtx_);
    for (int fd : clients_) {
        if (fd == from_fd) continue;
        if (!send_all(fd, line.c_str(), line.size())) {
            tslog::warn("server: broadcast send failed");
        }
    }
}
