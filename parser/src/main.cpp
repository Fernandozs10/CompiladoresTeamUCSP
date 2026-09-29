#include <iostream>
#include "parser.h"

// Uso: chocopy-parser [archivo_gramatica]   (por defecto compiladores.txt)
// Termina con 1 si hay conflictos y con 2 si no se pudo leer la gramática.
int main(int argc, char* argv[]) {
    Parser parser;
    std::string filename = (argc > 1) ? argv[1] : "compiladores.txt";

    std::cout << "Cargando gramática desde: " << filename << " ...\n" << std::endl;

    if (!parser.loadGrammar(filename)) {
        std::cerr << "Error al abrir el archivo de gramática." << std::endl;
        return 2;
    }

    parser.printGrammar();

    parser.computeFirstSets();              // 1. first()
    parser.printFirstSets();

    parser.buildCanonicalCollection();      // 5. coleccionCanonica() (usa closure y goto)
    parser.printCanonicalCollection();

    int conflicts = parser.fillTables();    // 6. llenarTablas()
    std::cout << "--- Tablas Action y Goto ---\n" << parser.tablesText() << std::endl;

    if (conflicts > 0) {
        parser.printConflicts();
        return 1;
    }
    return 0;
}
