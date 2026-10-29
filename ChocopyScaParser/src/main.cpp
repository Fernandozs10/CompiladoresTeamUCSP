#include "parser.h"
#include "scanner.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

static const std::set<std::string> DEFAULT_PANIC_NONTERMINALS = {"stmt", "simple_stmt", "block","def","class_body_def"};//expresion se saco para probar otros 

static void printErrors(const ParseResult& result) {
    for (const std::string& diagnostic : result.diagnostics) {
        std::cout << "ERROR - " << diagnostic << '\n';
    }
}

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

// Uso: chocopy-parser [archivo_gramatica] [--source archivo.py | --parse "ID ASSIGN INTEGER"] [--panic stmt,expr] [--details | --errors]
int main(int argc, char* argv[]) {
    Parser parser;
    std::string filename = "src/chocopy.txt";
    std::string parse_input;
    std::string source_filename;
    std::set<std::string> panic_nonterminals;
    bool should_parse = false;
    bool should_scan_source = false;
    bool should_parse_tokens = false;
    bool show_details_option = false;
    bool errors_only = false;

    const auto hasOptionValue = [&](const std::string& option, int index) {
        if (index + 1 >= argc || std::string(argv[index + 1]).rfind("--", 0) == 0) {
            std::cerr << "La opción " << option << " requiere un argumento.\n";
            return false;
        }
        return true;
    };

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--parse") {
            if (!hasOptionValue(argument, i)) return 2;
            parse_input = argv[++i];
            should_parse = true;
            should_parse_tokens = true;
        } else if (argument == "--source") {
            if (!hasOptionValue(argument, i)) return 2;
            source_filename = argv[++i];
            should_parse = true;
            should_scan_source = true;
        } else if (argument == "--panic") {
            if (!hasOptionValue(argument, i)) return 2;
            std::stringstream symbols(argv[++i]);
            std::string symbol;
            while (std::getline(symbols, symbol, ',')) {
                if (!symbol.empty()) panic_nonterminals.insert(symbol);
            }
        } else if (argument == "--details") {
            show_details_option = true;
        } else if (argument == "--errors") {
            errors_only = true;
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

    const bool show_parser_details = !errors_only && (!should_parse || show_details_option);
    if (show_parser_details) {
        std::cout << "Cargando gramática desde: " << filename << " ...\n" << std::endl;
    }

    if (!parser.loadGrammar(filename)) {
        std::cerr << "Error al abrir el archivo de gramática." << std::endl;
        return 2;
    }

    if (show_parser_details) parser.printGrammar();
    parser.computeFirstSets();
    if (show_parser_details) parser.printFirstSets();
    parser.buildCanonicalCollection();
    if (show_parser_details) parser.printCanonicalCollection();

    const int conflicts = parser.fillTables();
    if (show_parser_details) {
        std::cout << "--- Tablas Action y Goto ---\n" << parser.tablesText() << std::endl;
    }

    if (conflicts > 0) {
        if (!errors_only) parser.printConflicts();
        return 1;
    }

    if (!should_parse) return 0;

    std::vector<std::string> tokens;
    std::vector<ParseTokenLocation> token_locations;
    std::size_t lexical_errors = 0;
    if (should_scan_source) {
        if (!errors_only) {
            std::cout << "Escaneando archivo fuente: " << source_filename << '\n';
        }
        std::ifstream source_file(source_filename, std::ios::binary);
        if (!source_file) {
            std::cerr << "Error al abrir el archivo fuente: " << source_filename << '\n';
            return 2;
        }

        const std::string source_bytes(
            (std::istreambuf_iterator<char>(source_file)),
            std::istreambuf_iterator<char>());
        const CodeStr decoded_source = utf8Decode(source_bytes);
        std::vector<std::string> source_lines;
        for (const auto& line : physicalLines(decoded_source)) {
            source_lines.push_back(utf8Encode(line.second));
        }
        Scanner scanner(decoded_source, false, errors_only);
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
            std::string source_line;
            if (scanned_token.line > 0 &&
                static_cast<std::size_t>(scanned_token.line) <= source_lines.size()) {
                source_line = source_lines[static_cast<std::size_t>(scanned_token.line) - 1];
            }
            token_locations.push_back(ParseTokenLocation{
                scanned_token.line, scanned_token.col, source_line});
        }
        if (!scanned_tokens.empty() && scanned_tokens.back().type == "EOF") {
            const Token& eof = scanned_tokens.back();
            std::string source_line;
            if (!source_lines.empty()) source_line = source_lines.back();
            token_locations.push_back(ParseTokenLocation{eof.line, eof.col, source_line});
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
        const ParseResult result = parser.parseWithPanicRecovery(
            tokens, panic_nonterminals, token_locations);
        if (errors_only) printErrors(result);
        else printTrace(result);
        return lexical_errors > 0 || result.aborted || result.syntax_errors > 0 ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "Error al analizar: " << error.what() << '\n';
        return 2;
    }
}
