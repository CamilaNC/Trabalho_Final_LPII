# Arquitetura — Tema A (Chat TCP)

## Mapeamento Requisitos → Código

-> Servidor TCP concorrente aceitando múltiplos clientes: Implementado na classe ChatServer, especificamente no método accept_loop(). Um loop while chama a função ::accept() continuamente para receber novas conexões de clientes.

-> Cada cliente atendido por uma thread:	No método ChatServer::accept_loop(), para cada nova conexão aceita, uma nova thread é criada para gerenciar o cliente: std::thread(&ChatServer::client_worker, this, cfd).detach();.

-> Mensagens retransmitidas para os demais (broadcast):	A função ChatServer::broadcast_line(const std::string& line, int from_fd) é responsável por essa lógica. Ela itera sobre a lista de clientes (protegida por mutex) e envia a mensagem para todos, exceto o remetente original.

-> Logging concorrente de mensagens (usando libtslog):	A biblioteca libtslog foi integrada e inicializada nos arquivos server_main.cpp e cliente_main.cpp. Chamadas como tslog::info(...) e tslog::error(...) são usadas em todo o projeto para registrar eventos de forma segura.

-> Cliente CLI para conectar, enviar e receber mensagens:	O projeto cliente_cli implementa esta funcionalidade. O main gerencia a entrada do usuário via std::cin, enquanto uma thread separada (ChatClient::recv_loop) recebe mensagens do servidor e as exibe no console.

->Proteção de estruturas compartilhadas:	A estrutura de dados principal, std::vector<Client> clients_ na classe ChatServer, é protegida por um std::mutex mtx_. Todos os acessos (leitura, escrita, remoção) a este vetor são encapsulados por um std::lock_guard.

---

# Arquitetura — Tema A (Chat TCP)

## 1) Diagrama de sequência (visão cliente-servidor)
sequenceDiagram
    actor C1 as Cliente 1
    actor C2 as Cliente 2
    participant S as ChatServer

    activate C1
    C1->>S: TCP connect()
    activate S
    S-->>C1: Conexão aceita
    S-->>C1: "Bem-vindo ao chat! ..."
    C1->>S: /nick Joao
    S-->>C1: "Seu nick agora é Joao"
    Note right of S: Atualiza lista de clientes<br/>e faz broadcast
    S-->>C1: "* Joao entrou no chat"
    deactivate C1
    
    activate C2
    C2->>S: TCP connect()
    S-->>C2: Conexão aceita
    S-->>C2: "Bem-vindo ao chat! ..."
    C2->>S: /nick Maria
    S-->>C2: "Seu nick agora é Maria"
    Note right of S: Atualiza lista de clientes<br/>e faz broadcast
    S-->>C2: "* Maria entrou no chat"
    S-->>C1: "* Maria entrou no chat"
    deactivate C2

    activate C1
    C1->>S: "Oi!"
    Note right of S: Broadcast para todos<br/>exceto C1
    S-->>C2: "Joao: Oi!"
    deactivate C1

    activate C2
    C2->>S: /quit
    Note right of S: Remove C2 da lista<br/>e notifica os demais
    S-->>C1: "* Maria saiu do chat"
    deactivate S
    deactivate C2

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

## 3) Diagrama de classes 
classDiagram
  class ChatServer {
    - int listen_fd_
    - int port_
    - atomic<bool> running_
    - mutex mtx_
    - vector<int> clients_
    - vector<string> history_
    - static const size_t HISTORY_MAX
    + ChatServer(int port)
    + ~ChatServer()
    + bool start()
    + void stop()
    + size_t client_count() const
    - void accept_loop()
    - void client_worker(int client_fd)
    - void broadcast_line(string line, int from_fd)
    - void send_history_to(int client_fd)
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
  }

  ChatServer ..> tslog : usa (logging; history, nick, broadcast)
  ChatClient ..> tslog : usa (connect/disconnect/errors)

## 4) Concorrência do logger
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


## 5) Diagrama de sequência — Broadcast seguro (snapshot da lista)

sequenceDiagram
  participant Worker as client_worker (thread)
  participant Server as ChatServer
  participant B as Blocking area (mtx_ & history_)
  participant Net as network sends (send_all)

  Worker->>Server: recebe linha do cliente
  Server->>B: lock(mtx_) ; append(history_)
  B-->>Server: snapshot clients (copy list)
  Server->>B: unlock(mtx_)
  Server->>Net: para cada fd em snapshot -> send_all(fd, msg)
  Net-->>Server: sucesso/falha (log e remoção posterior se necessário)
