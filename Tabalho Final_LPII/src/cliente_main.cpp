#include "cliente.hpp"
#include "tslog.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char **argv) {
    std::string host = "127.0.0.1";
    int port = 9000;
    std::string nick = "";

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) nick = argv[3]; 

    tslog::Config cfg;
    cfg.file = "logs/client.log";
    cfg.mirror_stdout = true;
    cfg.level = tslog::INFO;
    tslog::init(cfg);

    tslog::info("client_cli: iniciado");

    ChatClient cli(host, port);
    if (!cli.connect_to_server()) {
        tslog::error("client_cli: falha ao conectar");
        tslog::shutdown();
        return 1;
    }

    if (!nick.empty()) {
        // envie como comando 'nick <nome>' para o servidor
        cli.send_line("nick " + nick);
    }

    std::cout << "Conectado em " << host << ":" << port
              << ". Digite mensagens e ENTER. /nick <nome> para alterar apelido. /quit para sair.\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.rfind("/nick ", 0) == 0) {
            // enviar apenas "nick <nome>" para o servidor
            std::string name = line.substr(6);
            cli.send_line("nick " + name);
            continue;
        }
        if (line == "/quit") {
            tslog::info("client_cli: comando /quit recebido");
            cli.send_line("/quit"); // informe ao servidor
            break;
        }
        if (!cli.send_line(line)) break;
    }

    cli.disconnect();
    tslog::info("client_cli: encerrado");

    tslog::shutdown();
    return 0;
}