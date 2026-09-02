# Scanner de ChocoPy v2.2 en C++
#
#   make            compila chocopy-scanner y run_tests
#   make test       compila y ejecuta la bateria de pruebas
#   make clean      borra los binarios y objetos

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS  ?=

# Algunas versiones de GCC < 9 necesitan enlazar stdc++fs para <filesystem>.
FSLIB    ?=

BIN      := bin
SRC      := src
SOURCES  := $(SRC)/tokens.cpp $(SRC)/scanner.cpp
SCANNER  := $(BIN)/chocopy-scanner
TESTS    := $(BIN)/run_tests

.PHONY: all test clean

all: $(SCANNER) $(TESTS)

$(BIN):
	@mkdir -p $(BIN)

$(SCANNER): $(SOURCES) $(SRC)/main.cpp $(SRC)/scanner.h $(SRC)/tokens.h | $(BIN)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(SRC)/main.cpp -o $@ $(LDFLAGS) $(FSLIB)

$(TESTS): $(SOURCES) tests/run_tests.cpp $(SRC)/scanner.h $(SRC)/tokens.h | $(BIN)
	$(CXX) $(CXXFLAGS) $(SOURCES) tests/run_tests.cpp -o $@ $(LDFLAGS) $(FSLIB)

test: $(TESTS)
	./$(TESTS) tests

clean:
	rm -rf $(BIN)
