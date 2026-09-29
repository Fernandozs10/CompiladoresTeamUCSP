# make        compila build/chocopy-parser y build/run_tests
# make test   ejecuta la batería de pruebas
# make run    genera las tablas de src/compiladores.txt
# make clean  borra build/

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

all: build/chocopy-parser build/run_tests

build/chocopy-parser: src/main.cpp src/parser.cpp src/parser.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/parser.cpp src/main.cpp -o $@

build/run_tests: test/run_tests.cpp src/parser.cpp src/parser.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -Isrc src/parser.cpp test/run_tests.cpp -o $@

# Además comprueba que ante un conflicto la herramienta termina con error
test: all
	./build/run_tests test/cases
	! ./build/chocopy-parser test/cases/conflicto_rr.txt > /dev/null

run: build/chocopy-parser
	./build/chocopy-parser src/compiladores.txt

clean:
	rm -rf build

.PHONY: all test run clean
