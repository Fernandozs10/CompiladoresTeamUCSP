
#ifndef CHOCOPY_TOKENS_H
#define CHOCOPY_TOKENS_H

#include <cstdint>
#include <string>
#include <vector>

using CodeStr = std::u32string;


extern const std::vector<std::string> KEYWORDS;
bool isKeyword(const std::string& word);

// --- Operadores y delimitadores (Reference 3.5) --------------------------
struct OperatorInfo {
    const char* name;
    const char* lexeme;
};
extern const std::vector<OperatorInfo> OPERATORS;

// --- Tokens de estructura de linea (Reference 3.1) -----------------------
extern const std::vector<std::string> STRUCTURE;

// Que clase de valor lleva un token.
enum class ValueKind { None, Int, Str };

// Un token: <categoria, lexema> con su posicion (1-indexada).
struct Token {
    std::string type;
    ValueKind kind = ValueKind::None;
    long long intValue = 0;
    CodeStr strValue;
    int line = 0;
    int col = 0;

    static Token plain(std::string type, int line, int col);
    static Token integer(std::string type, long long value, int line, int col);
    static Token text(std::string type, CodeStr value, int line, int col);

    // Equivalente a Token.__str__ de la version en Python.
    std::string str() const;
};

// Un error lexico, con su posicion.
struct LexErrorInfo {
    std::string message;
    int line = 0;
    int col = 0;

    // "linea:columna: error lexico: mensaje"
    std::string what() const;
};

// Formato de texto determinista: primero los tokens, luego los errores.
// Lo usan tanto el CLI (main.cpp) como la bateria de pruebas.
std::string render(const std::vector<Token>& tokens,
                   const std::vector<LexErrorInfo>& errors);

// --- utilidades de texto -------------------------------------------------

// Decodifica UTF-8 a puntos de codigo. Las secuencias invalidas se
// sustituyen por U+FFFD, igual que Python con errors="replace".
CodeStr utf8Decode(const std::string& bytes);
std::string utf8Encode(const CodeStr& s);
std::string utf8Encode(char32_t c);

// Reproduce repr() de Python para cadenas (comillas simples salvo que la
// cadena contenga ' y no ", escapes \\ \n \t \r y \xNN para controles).
std::string pyRepr(const CodeStr& s);

// Escapa una cadena como json.dumps(..., ensure_ascii=False).
std::string jsonEscape(const CodeStr& s);

#endif  // CHOCOPY_TOKENS_H
