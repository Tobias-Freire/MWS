# Variáveis
CXX = g++
CXXFLAGS = -Wall -O2
SRC = src/main.cpp src/implementations/*.cpp
BIN = main

# Compilar
all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

# Rodar
run: $(BIN)
	./$(BIN)

# Limpar
clean:
	rm -f $(BIN)

.PHONY: all run clean