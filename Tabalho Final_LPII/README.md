# Etapa 1 — libtslog (Trabalho Final de Programação Concorrente)

Este repositório contém a implementação da **Etapa 1** e **Etapa 2** do tema A (Chat TCP multiusuário):  
uma biblioteca de logging concorrente (`libtslog`) e um programa de teste de stress (`tslog_stress`).

---

## Como compilar e executar (Etapa 1)

No terminal, dentro do diretório do projeto:

make clean
make run-stress

Isso vai:

    Compilar a biblioteca (src/tslog.cpp)

    Compilar o exemplo (exemplos/tslog_stress.cpp)

    Executar o programa de stress (bin/tslog_stress)

    Gerar logs em logs/app.log e espelhar no terminal

    Obs.: as pastas bin/ e logs/ são criadas apenas após a execuçã

## Validação

O teste cria 8 threads, cada uma escrevendo 5000 mensagens → total esperado = 40000 linhas.

Verifique:

# 1. Total de linhas do log
wc -l logs/app.log
# deve retornar 40000

# 2. Todas as threads produtoras
grep -o 'thread=[0-9]\+' logs/app.log | sort -u
# deve listar thread=0 até thread=7

# 3. Diversidade de thread IDs capturados
grep -o 'tid=[0-9]\+' logs/app.log | sort -u | wc -l
# deve retornar 8 (uma por thread)

## Arquitetura do logger

    Produtores: múltiplas threads chamando tslog::debug/info/warn/error.

    Monitor (BlockingQueue): fila thread-safe que armazena os registros.

    Consumidor (Worker thread): única thread que escreve no arquivo e no stdout.


(Esquema simplificado: Threads produtoras → BlockingQueue → Worker thread → arquivo/stdout)

## Etapa 2 — Protótipo CLI de Comunicação

A Etapa 2 implementa um protótipo cliente/servidor TCP mínimo com logging integrado:

# Servidor (server_cli)
    Aceita múltiplos clientes, cria uma thread por conexão (detach) e retransmite mensagens recebidas (broadcast).
    Registra eventos com libtslog (conexão, desconexão, mensagens).

# Cliente (cliente_cli)
    Conecta ao servidor, envia mensagens digitadas no terminal e exibe mensagens recebidas.
    Suporta comando /quit para encerrar.

# Scripts de teste

    exemplos/run_clients.sh → dispara múltiplos clientes bots que enviam mensagens automáticas.

    teste/teste_e2e.sh → teste end-to-end: sobe servidor, dispara bots, valida logs e garante que o broadcast funciona.

## Como compilar e executar (Etapa 2)

1. Compilar tudo: 
make all
2. Iniciar o servidor: 
make run-server
3. Conectar um cliente interativo: 
make run-cliente
    Obs.: Digite mensagens e pressione ENTER.Use /quit para encerrar.
4. Rodar clientes automáticos (bots): 
./exemplos/run_clients.sh 5
5. Executar testes end-to-end:
make test-e2e

## Próximos passos (Etapa 3)

Implementar suporte a apelidos (/nick) e prefixar mensagens no broadcast.

Suporte a /quit tratado pelo servidor (remover cliente limpo).

Melhorias de UX e robustez no cliente/servidor.