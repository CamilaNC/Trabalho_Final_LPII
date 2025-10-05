# Projeto Final - Programação Concorrente (C/C++)

Este projeto implementa as três etapas do trabalho final da disciplina **LPII - Programação Concorrente**.  
O sistema demonstra uso de **concorrência**, **comunicação entre processos** e **sincronização** através de:

- **Etapa 1:** Logger concorrente (`tslog`) com fila bloqueante.
- **Etapa 2:** Servidor e cliente TCP simples.
- **Etapa 3:** Extensão do chat com comandos `/nick` e `/quit`.

---

## Organização do Projeto

.
├── src/                             # Código-fonte principal
│ ├── tslog.cpp/.hpp                 # Logger concorrente (Etapa 1)
│ ├── blocking_queue.hpp
│ ├── server.cpp/.hpp
│ ├── server_main.cpp
│ ├── cliente.cpp/.hpp
│ └── cliente_main.cpp
├── exemplos/
│ └── tslog_stress.cpp               # Programa de teste do logger (Etapa 1)
├── logs/                            # Logs gerados em tempo de execução
├── bin/                             # Binários compilados
├── teste/
│ └── teste_e2e.sh                   # Script de teste end-to-end
├── Makefile                         # Automação de compilação/execução
└── README.md

---

## Compilação
Para compilar todas as etapas de uma vez:
  make all

Para limpar binários e logs:
  make clean

##  Execução por Etapa

--> Etapa 1 - Logger concorrente
Compilar e executar o programa de stress test:
  make run-stress

-Saída no terminal mostrando várias threads escrevendo no logger.
-Arquivo de log gerado em logs/app.log.

--> Etapa 2 - Cliente/Servidor TCP básico
Compilar servidor e cliente:
  make server
  make cliente

Executar servidor (porta 9000 por padrão):
  make run-server

Em outro terminal, executar cliente:
  make run-cliente

-O cliente conecta ao servidor e permite enviar mensagens simples.

--> Etapa 3 - Chat com comandos /nick e /quit
O mesmo binário da Etapa 2 já suporta os comandos:
- /nick <nome> → define ou altera o apelido.
- /quit → desconecta o cliente do servidor.

Execução idêntica à Etapa 2:
  make run-server
  make run-cliente


## Testes Automáticos
Rodar os testes end-to-end (executa servidor e clientes simulados):
  make test-e2e
  ou
  ./exemplos/run_clients.sh 5

- Logs ficam registrados em logs/.
