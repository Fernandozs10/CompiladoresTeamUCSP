#include <iostream>
#include "parser.h"

// Uso: chocopy-parser [archivo_gramatica] [--no-cache]   (por defecto compiladores.txt)
// Las tablas se guardan en <archivo_gramatica>.lr1cache y se reutilizan mientras
// la gramática no cambie; --no-cache las calcula siempre y no toca ese archivo.
// Termina con 1 si hay conflictos y con 2 si no se pudo leer la gramática.
int main(int argc, char* argv[]) {
    Parser parser;
    std::string filename = "compiladores.txt";
    bool use_cache = true;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--no-cache") use_cache = false;
        else filename = argument;
    }
    const std::string cache_file = filename + ".lr1cache";

    std::cout << "Cargando gramática desde: " << filename << " ...\n" << std::endl;

    if (!parser.loadGrammar(filename)) {
        std::cerr << "Error al abrir el archivo de gramática." << std::endl;
        return 2;
    }

    parser.printGrammar();

    parser.computeFirstSets();              // 1. first()
    parser.printFirstSets();

    // Los mensajes de la caché van a stderr para no cambiar la salida estándar
    const bool from_cache = use_cache && parser.loadTablesCache(cache_file);
    if (from_cache) {
        std::cerr << "Tablas cargadas desde la caché: " << cache_file << std::endl;
    } else {
        parser.buildCanonicalCollection();  // 5. coleccionCanonica() (usa closure y goto)
    }
    parser.printCanonicalCollection();

    int conflicts = from_cache ? 0 : parser.fillTables();   // 6. llenarTablas()
    std::cout << "--- Tablas Action y Goto ---\n" << parser.tablesText() << std::endl;

    if (conflicts > 0) {
        parser.printConflicts();
        return 1;
    }
    if (use_cache && !from_cache && parser.saveTablesCache(cache_file)) {
        std::cerr << "Tablas guardadas en la caché: " << cache_file << std::endl;
    }
    return 0;
}
