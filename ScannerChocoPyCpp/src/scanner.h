// Analizador lexico (scanner) de ChocoPy v2.2, escrito a mano.
//
// Modelo de ejecucion (charla "Scanners" del curso):
//
//   * recorrido caracter a caracter;
//   * "maximal munch": se acepta el lexema legal mas largo leyendo de
//     izquierda a derecha (Reference 3);
//   * encima del reconocedor de tokens hay una capa que maneja la
//     estructura de lineas y emite NEWLINE / INDENT / DEDENT
//     (Reference 3.1).
//
// En ChocoPy una LINEA LOGICA es exactamente una LINEA FISICA
// (Reference 3.1.2: "A logical line is a physical line that contains at
// least one token that is not whitespace or comments"). No existe union
// de lineas, ni implicita (parentesis/corchetes) ni explicita
// (backslash), a diferencia de Python. Por eso toda linea fisica que no
// sea vacia ni solo-comentario produce exactamente un NEWLINE.
//
// El scanner es tolerante a errores: por defecto reporta cada error
// lexico y continua (para detectar varios en una sola pasada). Con
// strict = true aborta lanzando LexError en el primero.

#ifndef CHOCOPY_SCANNER_H
#define CHOCOPY_SCANNER_H

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "tokens.h"

// 2**31 - 1  (Reference 3.4.2)
constexpr long long INT_MAX_CHOCOPY = 2147483647LL;

// Excepcion lanzada en modo estricto.
class LexError : public std::runtime_error {
public:
    LexError(std::string message, int line, int col)
        : std::runtime_error(LexErrorInfo{message, line, col}.what()),
          info{std::move(message), line, col} {}

    LexErrorInfo info;
};

// Parte la fuente en lineas fisicas aceptando LF, CRLF y CR
// (Reference 3.1.1). Devuelve pares (numero_de_linea, texto_sin_fin).
// El fin de entrada tambien termina la ultima linea fisica.
std::vector<std::pair<int, CodeStr>> physicalLines(const CodeStr& src);

class Scanner {
public:
    explicit Scanner(CodeStr source, bool strict = false);

    // Ejecuta el analisis completo y devuelve la lista de tokens.
    const std::vector<Token>& tokenize();

    const std::vector<Token>& tokens() const { return tokens_; }
    const std::vector<LexErrorInfo>& errors() const { return errors_; }

private:
    // -- utilidades internas ----------------------------------------
    void error(const std::string& msg, int line, int col);
    void emit(Token t);

    // -- estructura de lineas ---------------------------------------
    void handleIndent(int indent, int lineno);

    // -- reconocimiento de tokens dentro de una linea ---------------
    void scanLine(int lineno, const CodeStr& text, size_t i);
    size_t scanOperator(int lineno, const CodeStr& text, size_t i);
    size_t scanNumber(int lineno, const CodeStr& text, size_t i);
    size_t scanString(int lineno, const CodeStr& text, size_t i);

    CodeStr source_;
    bool strict_;
    std::vector<Token> tokens_;
    std::vector<LexErrorInfo> errors_;
    std::vector<int> indents_;  // pila de indentacion (Reference 3.1.5)
};

#endif  // CHOCOPY_SCANNER_H
