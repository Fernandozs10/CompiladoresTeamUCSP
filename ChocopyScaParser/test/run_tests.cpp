#include "parser.h"

#include <cstdio>
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

    // Recuperación en modo pánico con la gramática y ejemplos del documento.
    Parser recovery_parser;
    if (!recovery_parser.loadGrammar(dir + "panic_recovery.txt")) {
        std::cerr << "No se pudo cargar la gramática de recuperación.\n";
        return 1;
    }
    recovery_parser.computeFirstSets();
    recovery_parser.buildCanonicalCollection();
    if (recovery_parser.fillTables() != 0) {
        std::cerr << "La gramática de recuperación contiene conflictos LR.\n";
        return 1;
    }

    const ParseResult statement_recovery = recovery_parser.parseWithPanicRecovery(
        { "id", "=", "id", "+", "+", "id", ";", "id", "=", "num" },
        { "S" });
    if (!statement_recovery.accepted || statement_recovery.aborted ||
        statement_recovery.syntax_errors != 1 || statement_recovery.discarded_tokens == 0 ||
        statement_recovery.trace.empty()) {
        std::cerr << "Fallo: la recuperación con N={S} debía descartar la expresión errónea.\n";
        return 1;
    }
    bool trace_has_error = false;
    bool trace_has_recovery = false;
    for (const ParseTraceStep& step : statement_recovery.trace) {
        trace_has_error = trace_has_error || step.kind == "error";
        trace_has_recovery = trace_has_recovery || step.kind == "recovery";
    }
    if (!trace_has_error || !trace_has_recovery) {
        std::cerr << "Fallo: la traza debía mostrar el error y los pasos de recuperación.\n";
        return 1;
    }

    const ParseResult expression_recovery = recovery_parser.parseWithPanicRecovery(
        { "id", "=", "id", "+", "+", "id", ";", "id", "=", "num" },
        { "S", "E", "T" });
    if (!expression_recovery.accepted || expression_recovery.aborted ||
        expression_recovery.syntax_errors != 1 || expression_recovery.discarded_tokens != 0) {
        std::cerr << "Fallo: la recuperación con N={S,E,T} debía conservar la entrada.\n";
        return 1;
    }

    const ParseResult bad_anchor_recovery = recovery_parser.parseWithPanicRecovery(
        { "while", "(", "id", "id", "=", "num", ";", "id", "=", "num" },
        { "E", "T" });
    if (!bad_anchor_recovery.aborted || bad_anchor_recovery.accepted ||
        bad_anchor_recovery.syntax_errors < 2) {
        std::cerr << "Fallo: una recuperación sin progreso debía abortar al llegar al fin.\n";
        return 1;
    }
    std::cout << "PASS  panicRecovery\n";

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
        if (!p.loadGrammar(dir + name + ".txt")) {
            std::cout << "FAIL  " << name << "\n--- esperado ---\n" << "<archivo no encontrado>\n"
                      << "--- obtenido ---\n" << "No se pudo cargar la gramática.\n";
            failed++;
            continue;
        }

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
