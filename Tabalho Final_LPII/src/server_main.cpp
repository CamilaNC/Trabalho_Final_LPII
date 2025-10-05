#include "server.hpp"
#include "tslog.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
    int port = 9000;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    tslog::Config cfg;
    cfg.file = "logs/server.log";
    cfg.mirror_stdout = true;   
    cfg.level = tslog::INFO;
    tslog::init(cfg);

    std::cout << "=== Servidor de Chat (Etapa 3) ===\n";
    std::cout << "Porta: " << port << std::endl;

    ChatServer server(port);
    if (!server.start()) {
        std::cerr << "Erro ao iniciar o servidor" << std::endl;
        tslog::shutdown();
        return 1;
    }

    tslog::shutdown();
    return 0;
}
