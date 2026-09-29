#include "Parser.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: run_tests <archivo_gramatica>\n";
        return 2;
    }

    Parser parser;
    if (!parser.loadGrammar(argv[1])) {
        std::cerr << "No se pudo cargar la gramática de prueba: " << argv[1] << '\n';
        return 1;
    }

    parser.computeFirstSets();

    const auto first_ab = parser.computeFirstOfString({"a", "B"});
    if (first_ab != std::set<std::string>{"a"}) {
        std::cerr << "Fallo: FIRST(a B) debía ser {a}.\n";
        return 1;
    }

    const auto first_b = parser.computeFirstOfString({"B"});
    if (first_b != std::set<std::string>{"b", "epsilon"}) {
        std::cerr << "Fallo: FIRST(B) debía ser {b, epsilon}.\n";
        return 1;
    }

    const auto first_empty = parser.computeFirstOfString({});
    if (first_empty != std::set<std::string>{"epsilon"}) {
        std::cerr << "Fallo: FIRST(cadena vacía) debía ser {epsilon}.\n";
        return 1;
    }

    std::cout << "Todas las pruebas del parser pasaron.\n";
    return 0;
}
