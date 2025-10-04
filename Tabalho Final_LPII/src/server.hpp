#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

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

    int listen_fd_ = -1;
    int port_;
    std::atomic<bool> running_{false};

    mutable std::mutex mtx_;
    std::vector<int> clients_;
};
