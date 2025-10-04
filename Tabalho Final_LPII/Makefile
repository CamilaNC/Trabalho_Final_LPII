CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Isrc

SRC = src/tslog.cpp
OBJ = $(SRC:.cpp=.o)

all: build

build: $(OBJ)
	@echo "built libtslog"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-stress: build
	mkdir -p logs bin
	$(CXX) $(CXXFLAGS) exemplos/tslog_stress.cpp src/tslog.cpp -o bin/tslog_stress
	./bin/tslog_stress

clean:
	rm -rf *.o src/*.o bin logs

.PHONY: all build run-stress clean
