# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17 -pthread

# Diretórios
SRC_DIR = src
IMPL_DIR = $(SRC_DIR)/implementations
HDR_DIR = $(SRC_DIR)/headers

# Arquivos fonte
SRC = $(SRC_DIR)/main.cpp \
      $(SRC_DIR)/server.cpp \
      $(SRC_DIR)/client.cpp \
      $(IMPL_DIR)/conn.cpp \
      $(IMPL_DIR)/libtslog.cpp

# Binário final
BIN = program

# Diretório padrão de arquivos HTML
WWW_DIR = www

# Porta padrão e backlog
PORT = 9000
BACKLOG = 20

# ======================================
# Targets principais
# ======================================

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

# Executa o servidor
run-server: $(BIN)
	@mkdir -p $(WWW_DIR)
	@echo "<html><body><h1>Servidor HTTP rodando</h1></body></html>" > $(WWW_DIR)/index.html
	./$(BIN) server $(PORT) $(WWW_DIR) $(BACKLOG)

# Executa um cliente de teste
run-client: $(BIN)
	./$(BIN) client 127.0.0.1 $(PORT) /index.html

# Teste automático (1 servidor + múltiplos clientes)
test: $(BIN)
	@bash tests/run_clients.sh

# Limpa binários e logs
clean:
	rm -f $(BIN) log.txt

.PHONY: all run-server run-client clean test
