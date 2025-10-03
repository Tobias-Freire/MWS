# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17 -pthread

# Fontes
SRC = src/main.cpp \
      src/server.cpp \
      src/client.cpp \
      src/implementations/*.cpp

# Binário final
BIN = program

# Regra padrão
all: $(BIN)

# Como compilar
$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

# Rodar servidor (exemplo)
run-server: $(BIN)
	./$(BIN) server 9000

# Rodar cliente (exemplo)
run-client: $(BIN)
	./$(BIN) client 127.0.0.1 9000

# Limpar
clean:
	rm -f $(BIN)

.PHONY: all run-server run-client clean
