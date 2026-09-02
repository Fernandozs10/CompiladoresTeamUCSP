// CLI del scanner de ChocoPy.
//
// Uso:
//     chocopy-scanner programa.py            # tokens en texto
//     chocopy-scanner programa.py --json     # tokens en JSON
//     chocopy-scanner programa.py --strict   # abortar al primer error

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "scanner.h"
#include "tokens.h"

namespace {

void usage() {
    std::cerr
        << "uso: chocopy-scanner <archivo> [--json] [--strict]\n"
        << "\n"
        << "Analizador lexico (scanner) de ChocoPy v2.2\n"
        << "\n"
        << "  <archivo>   archivo fuente de ChocoPy\n"
        << "  --json      emitir los tokens en formato JSON\n"
        << "  --strict    detenerse en el primer error lexico\n";
}

// Lee el archivo completo en binario (sin traducir fines de linea, igual
// que open(..., newline="") en Python).
bool readFileBinary(const std::string& path, std::string* out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    *out = ss.str();
    return true;
}

// Reproduce json.dumps(..., ensure_ascii=False, indent=2).
std::string toJson(const std::vector<Token>& tokens) {
    if (tokens.empty()) return "[]";
    std::string out = "[\n";
    for (size_t k = 0; k < tokens.size(); ++k) {
        const Token& t = tokens[k];
        out += "  {\n";
        out += "    \"type\": " + jsonEscape(utf8Decode(t.type)) + ",\n";
        out += "    \"value\": ";
        switch (t.kind) {
            case ValueKind::None: out += "null"; break;
            case ValueKind::Int:  out += std::to_string(t.intValue); break;
            case ValueKind::Str:  out += jsonEscape(t.strValue); break;
        }
        out += ",\n";
        out += "    \"line\": " + std::to_string(t.line) + ",\n";
        out += "    \"col\": " + std::to_string(t.col) + "\n";
        out += (k + 1 == tokens.size()) ? "  }\n" : "  },\n";
    }
    out += "]";
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    // stdout se deja en modo TEXTO a proposito: en Windows la CRT traduce
    // \n -> \r\n igual que hace print() de Python, de modo que la salida
    // redirigida es byte a byte identica a la de la version en Python.
    std::string path;
    bool json = false;
    bool strict = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--json") {
            json = true;
        } else if (arg == "--strict") {
            strict = true;
        } else if (arg == "-h" || arg == "--help") {
            usage();
            return 0;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "argumento no reconocido: " << arg << "\n";
            usage();
            return 2;
        } else if (path.empty()) {
            path = arg;
        } else {
            std::cerr << "solo se admite un archivo de entrada\n";
            usage();
            return 2;
        }
    }

    if (path.empty()) {
        usage();
        return 2;
    }

    std::string bytes;
    if (!readFileBinary(path, &bytes)) {
        std::cerr << "no se pudo abrir el archivo: " << path << "\n";
        return 2;
    }

    Scanner sc(utf8Decode(bytes), strict);
    try {
        sc.tokenize();
    } catch (const LexError& e) {
        std::cerr << e.what() << "\n";
        return 2;
    }

    if (json) {
        std::cout << toJson(sc.tokens()) << "\n";
    } else {
        std::cout << render(sc.tokens(), sc.errors()) << "\n";
    }
    std::cout.flush();

    return sc.errors().empty() ? 0 : 1;
}
