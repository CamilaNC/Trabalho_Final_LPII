#pragma once
#include <string>
#include <thread>
#include <atomic>

class ChatClient {
public:
    ChatClient(std::string host, int port);
    ~ChatClient();

    bool connect_to_server();     
    void disconnect();            

    // I/O futuro (CLI): enviar e receber
    bool send_line(const std::string& line);

private:
    void recv_loop();             
    std::string host_;
    int port_;
    int sock_fd_ = -1;
    std::thread recv_th_;
    std::atomic<bool> running_{false};
};
