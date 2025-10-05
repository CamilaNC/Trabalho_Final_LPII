#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

struct Client {
    int fd;
    std::string nick;
};

class ChatServer {
public:
    explicit ChatServer(int port);
    ~ChatServer();

    bool start();     
    void stop();      

    int  port() const { return port_; }
    size_t client_count() const;

private:
    void accept_loop();                    
    void client_worker(int client_fd);     
    void broadcast_line(const std::string& line, int from_fd);
    Client* find_client(int fd);

    std::string nick_of(int fd) const;

    int listen_fd_ = -1;
    int port_;
    std::atomic<bool> running_{false};

    mutable std::mutex mtx_;
    std::vector<Client> clients_;
};
