# Etapa 1 — libtslog (Trabalho Final de Programação Concorrente)

Este repositório contém a implementação da **Etapa 1** do tema A (Chat TCP multiusuário):  
uma biblioteca de logging concorrente (`libtslog`) e um programa de teste de stress (`tslog_stress`).

---

## Como compilar e executar

No terminal, dentro do diretório do projeto:

make clean
make run-stress

Isso vai:

    Compilar a biblioteca (src/tslog.cpp)

    Compilar o exemplo (exemplos/tslog_stress.cpp)

    Executar o programa de stress (bin/tslog_stress)

    Gerar logs em logs/app.log e espelhar no terminal

    Obs.: as pastas bin/ e logs/ são criadas apenas após a execução

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

## Próximos passos (Etapa 2)
- Implementar o servidor TCP e integrar chamadas `tslog::info/warn/error`.
- Criar scripts de teste simulando múltiplos clientes.