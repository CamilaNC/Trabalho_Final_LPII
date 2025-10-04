#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

struct ClientConn {
    int fd = -1;
    std::thread th;
};

class ChatServer {
public:
    explicit ChatServer(int port);
    ~ChatServer();

    // ciclo de vida
    bool start();     
    void stop();      

    // estatísticas (para CLI futuramente)
    int  port() const { return port_; }
    size_t client_count() const;

private:
    void accept_loop();                    
    void client_worker(int client_fd);     
    void broadcast_line(const std::string& line, int from_fd);

    int listen_fd_ = -1;
    int port_;
    std::atomic<bool> running_{false};

    // lista de clientes protegida por mutex
    mutable std::mutex mtx_;
    std::vector<ClientConn> clients_;
};
