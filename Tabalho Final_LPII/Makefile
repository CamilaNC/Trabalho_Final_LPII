CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Isrc
LDFLAGS =
BIN_DIR = bin

LIBSRC = src/tslog.cpp
LIBOBJ = $(LIBSRC:.cpp=.o)

SRV_OBJS = src/server.cpp src/server_main.cpp $(LIBOBJ)
CLI_OBJS = src/cliente.cpp src/cliente_main.cpp $(LIBOBJ)

all: build server cliente

build: $(LIBOBJ)
	@mkdir -p $(BIN_DIR) logs
	@echo "libtslog compilada"

server: $(SRV_OBJS)
	@mkdir -p $(BIN_DIR) logs
	$(CXX) $(CXXFLAGS) $(SRV_OBJS) -o $(BIN_DIR)/server_cli $(LDFLAGS)

cliente: $(CLI_OBJS)
	@mkdir -p $(BIN_DIR) logs
	$(CXX) $(CXXFLAGS) $(CLI_OBJS) -o $(BIN_DIR)/cliente_cli $(LDFLAGS)

run-server: server
	./$(BIN_DIR)/server_cli 9000

run-cliente: cliente
	./$(BIN_DIR)/cliente_cli 127.0.0.1 9000

run-stress: build
	@mkdir -p $(BIN_DIR) logs
	$(CXX) $(CXXFLAGS) exemplos/tslog_stress.cpp src/tslog.cpp -o $(BIN_DIR)/tslog_stress
	./$(BIN_DIR)/tslog_stress

test-e2e: all
	@bash teste/teste_e2e.sh
.PHONY: test-e2e

clean:
	rm -rf *.o src/*.o $(BIN_DIR) logs

.PHONY: all build server cliente run-server run-cliente run-stress clean
