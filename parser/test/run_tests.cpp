#include "parser.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

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

    // Caché de tablas: lo guardado y recargado debe dar exactamente la misma tabla,
    // y una caché inválida nunca debe cargarse.
    const std::string cache_path = dir + "prueba.lr1cache";
    const auto check = [&failed](const std::string& name, bool ok) {
        std::cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
        if (!ok) failed++;
    };
    const auto build = [&dir](Parser& p, const std::string& name) {
        p.loadGrammar(dir + name + ".txt");
        p.computeFirstSets();
        p.buildCanonicalCollection();
        return p.fillTables();
    };
    const auto freshParser = [&dir](Parser& p, const std::string& name) {
        p.loadGrammar(dir + name + ".txt");
        p.computeFirstSets();
    };

    for (const std::string& name : cases) {
        Parser p;
        const int conflicts = build(p, name);
        const bool saved = p.saveTablesCache(cache_path);
        if (conflicts > 0) {
            check("cache no guarda conflictos: " + name, !saved);
            continue;
        }
        Parser q;
        freshParser(q, name);
        check("cache igual a la tabla: " + name,
              saved && q.loadTablesCache(cache_path) && clean(q.tablesText()) == clean(p.tablesText()));
    }

    Parser original;
    build(original, "parentesis");
    original.saveTablesCache(cache_path);
    const std::string valid = readFile(cache_path);

    Parser other;
    freshParser(other, "vacias");
    check("cache de otra gramatica no se carga", !other.loadTablesCache(cache_path));

    const auto loadsText = [&](const std::string& text) {
        std::ofstream(cache_path, std::ios::trunc) << text;
        Parser q;
        freshParser(q, "parentesis");
        return q.loadTablesCache(cache_path);
    };
    check("cache valida se carga", loadsText(valid));
    check("cache vacia no se carga", !loadsText(""));
    check("cache truncada no se carga", !loadsText(valid.substr(0, valid.size() / 2)));
    check("cache sin marca END no se carga", !loadsText(valid.substr(0, valid.rfind("END"))));
    std::string wrong_version = valid;
    wrong_version.replace(wrong_version.find(' ') + 1, 1, "9");
    check("cache de otra version no se carga", !loadsText(wrong_version));
    std::string out_of_range = valid;   // "3 List 5" pasa a "93 List 5": el estado 93 no existe
    out_of_range.insert(out_of_range.find('\n', out_of_range.find("\nGOTO ") + 1) + 1, "9");
    check("cache con estado fuera de rango no se carga", !loadsText(out_of_range));
    {
        Parser q;
        freshParser(q, "parentesis");
        check("cache inexistente no se carga", !q.loadTablesCache(dir + "no_existe.lr1cache"));
    }
    std::remove(cache_path.c_str());

    std::cout << (failed == 0 ? "\nTodas las pruebas pasaron.\n" : "\nHay pruebas que fallaron.\n");
    return failed == 0 ? 0 : 1;
}
