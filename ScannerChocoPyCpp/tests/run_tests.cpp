// Bateria de pruebas del scanner de ChocoPy.
//
// Recorre tests/cases/ y tests/errores/, ejecuta el scanner sobre cada
// archivo .py y compara la salida (tokens + errores) contra el archivo
// .tokens de referencia con el mismo nombre.
//
//     run_tests               # ejecutar la bateria
//     run_tests --generate    # (re)generar los .tokens esperados
//
// Se puede pasar la ruta de la carpeta tests/ como argumento; por defecto
// se busca junto al ejecutable y en ./tests.

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/scanner.h"
#include "../src/tokens.h"

namespace fs = std::filesystem;

namespace {

std::string readBinary(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Lee un archivo de texto normalizando los fines de linea, igual que el
// modo texto de Python (universal newlines). Necesario porque los .tokens
// generados en Windows pueden traer CRLF.
std::string readText(const fs::path& p) {
    const std::string raw = readBinary(p);
    std::string out;
    out.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\r') {
            if (i + 1 < raw.size() && raw[i + 1] == '\n') ++i;
            out += '\n';
        } else {
            out += raw[i];
        }
    }
    return out;
}

std::string rstripNewlines(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s;
}

std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> lines;
    std::string cur;
    for (char c : s) {
        if (c == '\n') {
            lines.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) lines.push_back(cur);
    return lines;
}

// Ejecuta el scanner sobre un archivo y devuelve la salida renderizada.
std::string scanFile(const fs::path& p) {
    Scanner sc(utf8Decode(readBinary(p)));
    sc.tokenize();
    return render(sc.tokens(), sc.errors());
}

// Diff unificado minimo basado en la subsecuencia comun mas larga.
void printUnifiedDiff(const std::vector<std::string>& expected,
                      const std::vector<std::string>& actual) {
    const size_t n = expected.size();
    const size_t m = actual.size();
    std::vector<std::vector<size_t>> lcs(n + 1,
                                         std::vector<size_t>(m + 1, 0));
    for (size_t i = n; i-- > 0;) {
        for (size_t j = m; j-- > 0;) {
            lcs[i][j] = (expected[i] == actual[j])
                            ? lcs[i + 1][j + 1] + 1
                            : std::max(lcs[i + 1][j], lcs[i][j + 1]);
        }
    }
    std::cout << "    --- esperado\n    +++ obtenido\n";
    size_t i = 0;
    size_t j = 0;
    while (i < n && j < m) {
        if (expected[i] == actual[j]) {
            std::cout << "     " << expected[i] << "\n";
            ++i;
            ++j;
        } else if (lcs[i + 1][j] >= lcs[i][j + 1]) {
            std::cout << "    -" << expected[i++] << "\n";
        } else {
            std::cout << "    +" << actual[j++] << "\n";
        }
    }
    while (i < n) std::cout << "    -" << expected[i++] << "\n";
    while (j < m) std::cout << "    +" << actual[j++] << "\n";
}

// Localiza la carpeta tests/ (junto al ejecutable, o relativa al cwd).
fs::path findTestsDir(const char* argv0, const std::string& override) {
    if (!override.empty()) return fs::path(override);
    std::vector<fs::path> candidates;
    if (argv0) {
        const fs::path exe = fs::absolute(fs::path(argv0)).parent_path();
        candidates.push_back(exe / "tests");
        candidates.push_back(exe / ".." / "tests");
        candidates.push_back(exe / ".." / ".." / "tests");
    }
    candidates.push_back(fs::current_path() / "tests");
    candidates.push_back(fs::current_path());
    for (const fs::path& c : candidates) {
        std::error_code ec;
        if (fs::is_directory(c / "cases", ec)) return fs::weakly_canonical(c, ec);
    }
    return fs::path("tests");
}

std::vector<fs::path> collectCases(const fs::path& testsDir) {
    std::vector<fs::path> out;
    for (const char* sub : {"cases", "errores"}) {
        const fs::path dir = testsDir / sub;
        std::error_code ec;
        if (!fs::is_directory(dir, ec)) continue;
        std::vector<fs::path> here;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".py") {
                here.push_back(entry.path());
            }
        }
        std::sort(here.begin(), here.end());
        out.insert(out.end(), here.begin(), here.end());
    }
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    bool generate = false;
    std::string testsOverride;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--generate") {
            generate = true;
        } else {
            testsOverride = arg;
        }
    }

    const fs::path testsDir = findTestsDir(argv[0], testsOverride);
    const std::vector<fs::path> cases = collectCases(testsDir);

    if (cases.empty()) {
        std::cerr << "no se encontraron casos de prueba en " << testsDir
                  << "\n";
        return 2;
    }

    if (generate) {
        for (const fs::path& py : cases) {
            fs::path out = py;
            out.replace_extension(".tokens");
            // Modo TEXTO a proposito: en Windows escribe CRLF igual que
            // open(..., "w") de Python, para que los .tokens generados por
            // una u otra implementacion sean el mismo archivo.
            std::ofstream f(out);
            f << scanFile(py) << "\n";
            std::cout << "escrito " << out.filename().string() << "\n";
        }
        return 0;
    }

    int total = 0;
    int passed = 0;
    for (const fs::path& py : cases) {
        ++total;
        const std::string name = py.filename().string();
        const std::string actual = rstripNewlines(scanFile(py));

        fs::path exp = py;
        exp.replace_extension(".tokens");
        if (!fs::exists(exp)) {
            std::cout << "FAIL  " << name << "  (falta "
                      << exp.filename().string() << ")\n";
            continue;
        }
        const std::string expected = rstripNewlines(readText(exp));

        if (actual == expected) {
            ++passed;
            std::cout << "PASS  " << name << "\n";
        } else {
            std::cout << "FAIL  " << name << "\n";
            printUnifiedDiff(splitLines(expected), splitLines(actual));
        }
    }

    std::cout << "\n" << passed << "/" << total << " pruebas superadas\n";
    return (passed == total) ? 0 : 1;
}
