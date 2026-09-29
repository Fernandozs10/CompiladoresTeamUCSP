#include "parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Uso: run_tests <carpeta_de_casos>
// Cada gramática <caso>.txt se compara con su tabla esperada <caso>.tabla

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Quita '\r' (archivos con CRLF) y los saltos de línea del final
static std::string clean(const std::string& text) {
    std::string out;
    for (char c : text) {
        if (c != '\r') out += c;
    }
    while (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: run_tests <carpeta_de_casos>\n";
        return 2;
    }
    std::string dir = std::string(argv[1]) + "/";

    // Pruebas de firstCadena
    Parser parser;
    if (!parser.loadGrammar(dir + "prueba1.txt")) {
        std::cerr << "No se pudo cargar la gramática de prueba: " << dir << "prueba1.txt\n";
        return 1;
    }
    parser.computeFirstSets();

    if (parser.computeFirstOfString({"a", "B"}) != std::set<std::string>{"a"}) {
        std::cerr << "Fallo: FIRST(a B) debía ser {a}.\n";
        return 1;
    }
    if (parser.computeFirstOfString({"B"}) != std::set<std::string>{"b", "epsilon"}) {
        std::cerr << "Fallo: FIRST(B) debía ser {b, epsilon}.\n";
        return 1;
    }
    if (parser.computeFirstOfString({}) != std::set<std::string>{"epsilon"}) {
        std::cerr << "Fallo: FIRST(cadena vacía) debía ser {epsilon}.\n";
        return 1;
    }
    std::cout << "PASS  firstCadena\n";

    // Pruebas de tablas
    const std::string cases[] = {
        "parentesis",      // tabla de referencia del enunciado
        "vacias",          // producciones vacías
        "prueba1",
        "compiladores",
        "postfija",        // se agrega S' -> S
        "conflicto_sr",    // shift/reduce
        "conflicto_rr",    // reduce/reduce
        "declaraciones",   // shift/reduce con producciones vacías
    };

    int failed = 0;
    for (const std::string& name : cases) {
        Parser p;
        p.loadGrammar(dir + name + ".txt");
        p.computeFirstSets();
        p.buildCanonicalCollection();
        p.fillTables();

        std::string expected = clean(readFile(dir + name + ".tabla"));
        std::string actual = clean(p.tablesText());

        if (actual == expected) {
            std::cout << "PASS  " << name << "\n";
        } else {
            std::cout << "FAIL  " << name << "\n--- esperado ---\n" << expected
                      << "\n--- obtenido ---\n" << actual << "\n";
            failed++;
        }
    }

    std::cout << (failed == 0 ? "\nTodas las pruebas pasaron.\n" : "\nHay pruebas que fallaron.\n");
    return failed == 0 ? 0 : 1;
}
