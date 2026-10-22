#include "parser.h"
#include "scanner.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

static const std::set<std::string> DEFAULT_PANIC_NONTERMINALS = {
    "stmt", "expr", "block"
};

static void printTrace(const ParseResult& result) {
    std::size_t states_width = std::string("Estados").size();
    std::size_t symbols_width = std::string("Símbolos").size();
    std::size_t input_width = std::string("Entrada").size();
    for (const ParseTraceStep& step : result.trace) {
        states_width = std::max(states_width, step.states.size());
        symbols_width = std::max(symbols_width, step.symbols.size());
        input_width = std::max(input_width, step.remaining_input.size());
    }

    std::cout << "\nEntrada: "
              << (result.trace.empty() ? "" : result.trace.front().remaining_input)
              << "\n\n--- Traza del análisis y recuperación ---\n";
    std::cout << std::left << std::setw(static_cast<int>(states_width + 2)) << "Estados"
              << std::setw(static_cast<int>(symbols_width + 2)) << "Símbolos"
              << std::setw(static_cast<int>(input_width + 2)) << "Entrada"
              << "Acción\n";
    std::cout << std::string(states_width + symbols_width + input_width + 12, '-') << '\n';
    for (const ParseTraceStep& step : result.trace) {
        std::cout << std::left << std::setw(static_cast<int>(states_width + 2)) << step.states
                  << std::setw(static_cast<int>(symbols_width + 2)) << step.symbols
                  << std::setw(static_cast<int>(input_width + 2)) << step.remaining_input
                  << step.action << '\n';
    }
    std::cout << '\n';
    for (const std::string& diagnostic : result.diagnostics) {
        std::cout << diagnostic << '\n';
    }
    std::cout << (result.accepted ? "Entrada aceptada" : "Análisis abortado")
              << " (" << result.syntax_errors << " error(es), "
              << result.discarded_tokens << " token(s) descartado(s)).\n";
}

// Uso: chocopy-parser [archivo_gramatica] [--source archivo.py | --parse "ID ASSIGN INTEGER"] [--panic stmt,expr]
int main(int argc, char* argv[]) {
    Parser parser;
    std::string filename = "src/chocopy.txt";
    std::string parse_input;
    std::string source_filename;
    std::set<std::string> panic_nonterminals;
    bool should_parse = false;
    bool should_scan_source = false;
    bool should_parse_tokens = false;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--parse" && i + 1 < argc) {
            parse_input = argv[++i];
            should_parse = true;
            should_parse_tokens = true;
        } else if (argument == "--source" && i + 1 < argc) {
            source_filename = argv[++i];
            should_parse = true;
            should_scan_source = true;
        } else if (argument == "--panic" && i + 1 < argc) {
            std::stringstream symbols(argv[++i]);
            std::string symbol;
            while (std::getline(symbols, symbol, ',')) {
                if (!symbol.empty()) panic_nonterminals.insert(symbol);
            }
        } else if (argument.rfind("--", 0) == 0) {
            std::cerr << "Opción desconocida o incompleta: " << argument << '\n';
            return 2;
        } else {
            filename = argument;
        }
    }

    if (should_scan_source && should_parse_tokens) {
        std::cerr << "Use --source o --parse, no ambos.\n";
        return 2;
    }

    std::cout << "Cargando gramática desde: " << filename << " ...\n" << std::endl;

    if (!parser.loadGrammar(filename)) {
        std::cerr << "Error al abrir el archivo de gramática." << std::endl;
        return 2;
    }

    parser.printGrammar();
    parser.computeFirstSets();
    parser.printFirstSets();
    parser.buildCanonicalCollection();
    parser.printCanonicalCollection();

    const int conflicts = parser.fillTables();
    std::cout << "--- Tablas Action y Goto ---\n" << parser.tablesText() << std::endl;

    if (conflicts > 0) {
        parser.printConflicts();
        return 1;
    }

    if (!should_parse) return 0;

    std::vector<std::string> tokens;
    std::size_t lexical_errors = 0;
    if (should_scan_source) {
        std::cout << "Escaneando archivo fuente: " << source_filename << '\n';
        std::ifstream source_file(source_filename, std::ios::binary);
        if (!source_file) {
            std::cerr << "Error al abrir el archivo fuente: " << source_filename << '\n';
            return 2;
        }

        const std::string source_bytes(
            (std::istreambuf_iterator<char>(source_file)),
            std::istreambuf_iterator<char>());
        Scanner scanner(utf8Decode(source_bytes));
        const std::vector<Token>& scanned_tokens = scanner.tokenize();
        for (std::size_t i = 0; i < scanned_tokens.size(); ++i) {
            const Token& scanned_token = scanned_tokens[i];
            if (scanned_token.type == "EOF") continue;

            const bool declaration_position =
                i == 0 || scanned_tokens[i - 1].type == "NEWLINE" ||
                scanned_tokens[i - 1].type == "INDENT" ||
                scanned_tokens[i - 1].type == "DEDENT" ||
                scanned_tokens[i - 1].type == "LPAREN" ||
                scanned_tokens[i - 1].type == "COMMA";
            const bool typed_identifier =
                scanned_token.type == "ID" && i + 1 < scanned_tokens.size() &&
                scanned_tokens[i + 1].type == "COLON" && declaration_position;
            tokens.push_back(typed_identifier ? "TYPED_ID" : scanned_token.type);
        }
        lexical_errors = scanner.errors().size();
        if (panic_nonterminals.empty()) {
            panic_nonterminals = DEFAULT_PANIC_NONTERMINALS;
        }
    } else {
        if (panic_nonterminals.empty()) {
            std::cerr << "Debe indicar al menos un no terminal con --panic.\n";
            return 2;
        }

        std::istringstream input_stream(parse_input);
        std::string token;
        while (input_stream >> token) tokens.push_back(token);
    }

    try {
        const ParseResult result = parser.parseWithPanicRecovery(tokens, panic_nonterminals);
        printTrace(result);
        return lexical_errors > 0 || result.aborted || result.syntax_errors > 0 ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "Error al analizar: " << error.what() << '\n';
        return 2;
    }
}
