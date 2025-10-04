# Arquitetura — Tema A (Chat TCP)

## Componentes
- **ChatServer**: aceita conexões (`accept_loop`), cria uma **thread por cliente** (`client_worker`),
  mantém **lista de clientes protegida por mutex** e faz **broadcast** das mensagens.
- **ChatClient**: conecta ao servidor, cria thread de recepção (`recv_loop`) e envia linhas pelo stdin.
- **libtslog**: biblioteca de logging thread-safe, já implementada na Etapa 1.

---

## 1) Diagrama de sequência (visão cliente-servidor) 
mermaid
sequenceDiagram
    participant C1 as Cliente 1
    participant C2 as Cliente 2
    participant S as ChatServer

    C1->>S: TCP connect
    C2->>S: TCP connect
    Note over S: accept_loop cria thread client_worker por cliente

    C1->>S: "Olá"
    S->>C2: broadcast "C1: Olá"
    C2->>S: "Oi!"
    S->>C1: broadcast "C2: Oi!"

## 2) Visão de componentes (projeto como um todo)
flowchart LR
  subgraph App["Aplicação (Tema A - Chat)"]
    Srv["ChatServer (header)"]
    Cli["ChatClient (header)"]
    LogAPI["libtslog::API (tslog.hpp)"]
  end

  subgraph Logger["libtslog (implementado na Etapa 1)"]
    Prod["Produtores (threads chamando tslog::log)"]
    BQ["BlockingQueue<LogRecord>"]
    Cons["Worker Thread (consome e escreve)"]
    Dest["Arquivo logs/app.log + stdout"]
  end

  Srv --> LogAPI
  Cli --> LogAPI
  LogAPI --> Prod
  Prod --> BQ
  BQ --> Cons
  Cons --> Dest

## 3) Diagrama de classes (headers da Etapa 1)
classDiagram
  class ChatServer {
    - int listen_fd_
    - int port_
    - atomic<bool> running_
    - mutex mtx_
    - vector<ClientConn> clients_
    + ChatServer(int port)
    + ~ChatServer()
    + bool start()
    + void stop()
    + int port() const
    + size_t client_count() const
    - void accept_loop()
    - void client_worker(int client_fd)
    - void broadcast_line(string line, int from_fd)
  }

  class ClientConn {
    + int fd
    + thread th
  }

  class ChatClient {
    - string host_
    - int port_
    - int sock_fd_
    - thread recv_th_
    - atomic<bool> running_
    + ChatClient(string host, int port)
    + ~ChatClient()
    + bool connect_to_server()
    + void disconnect()
    + bool send_line(string line)
    - void recv_loop()
  }

  class tslog {
    <<namespace>>
    + init(Config)
    + shutdown()
    + log(Level, const char*)
    + debug(const char*)
    + info(const char*)
    + warn(const char*)
    + error(const char*)
  }

  ChatServer --> ClientConn : compõe
  ChatServer ..> tslog : usa (logging)
  ChatClient ..> tslog : usa (logging)

## 4) Concorrência do logger (foco da Etapa 1)
sequenceDiagram
  participant T1 as Thread Produtora #1
  participant Tn as Thread Produtora #n
  participant Q as BlockingQueue<LogRecord>
  participant W as Worker Thread
  participant F as logs/app.log + stdout

  T1->>Q: push(LogRecord DEBUG/INFO/...)
  Tn->>Q: push(LogRecord ...)
  Note over Q: mutex + condvar; espera por itens
  W->>Q: pop() (bloqueante)
  Q-->>W: LogRecord
  W->>F: write(line) + flush
  Note over W: em shutdown(): drena fila e termina
