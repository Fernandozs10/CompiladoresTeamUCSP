#include "scanner.h"
#include <iostream>
#include <map>

namespace {

// Unicas secuencias de escape validas dentro de string (Reference 3.4.1).
const std::map<char32_t, char32_t>& escapes() {
    static const std::map<char32_t, char32_t> m = {
        {U'"', U'"'}, {U'n', U'\n'}, {U't', U'\t'}, {U'\\', U'\\'}};
    return m;
}

const std::map<CodeStr, std::string>& op2() {
    static const std::map<CodeStr, std::string> m = {
        {U"//", "INTDIV"}, {U"<=", "LE"}, {U">=", "GE"},
        {U"==", "EQ"},     {U"!=", "NE"}, {U"->", "ARROW"}};
    return m;
}

const std::map<char32_t, std::string>& op1() {
    static const std::map<char32_t, std::string> m = {
        {U'+', "PLUS"},     {U'-', "MINUS"},    {U'*', "MULT"},
        {U'%', "MOD"},      {U'<', "LT"},       {U'>', "GT"},
        {U'=', "ASSIGN"},   {U'(', "LPAREN"},   {U')', "RPAREN"},
        {U'[', "LBRACKET"}, {U']', "RBRACKET"}, {U',', "COMMA"},
        {U':', "COLON"},    {U'.', "DOT"}};
    return m;
}

bool isDigit(char32_t c) { return c >= U'0' && c <= U'9'; }

bool isIdentStart(char32_t c) {
    return c == U'_' || (c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z');
}

bool isIdentPart(char32_t c) { return isIdentStart(c) || isDigit(c); }

// La cadena tiene la forma de un identificador: [A-Za-z_][A-Za-z0-9_]*
bool isIdentifier(const CodeStr& s) {
    if (s.empty()) return false;
    if (!isIdentStart(s[0])) return false;
    for (size_t i = 1; i < s.size(); ++i) {
        if (!isIdentPart(s[i])) return false;
    }
    return true;
}

std::string asciiUpper(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    }
    return out;
}

}  // namespace

std::vector<std::pair<int, CodeStr>> physicalLines(const CodeStr& src) {
    std::vector<std::pair<int, CodeStr>> lines;
    const size_t n = src.size();
    size_t i = 0;
    size_t start = 0;
    int lineno = 1;

    while (i < n) {
        const char32_t c = src[i];
        if (c == U'\n') {
            lines.emplace_back(lineno, src.substr(start, i - start));
            i += 1;
            start = i;
            lineno += 1;
        } else if (c == U'\r') {
            lines.emplace_back(lineno, src.substr(start, i - start));
            i += (i + 1 < n && src[i + 1] == U'\n') ? 2 : 1;
            start = i;
            lineno += 1;
        } else {
            i += 1;
        }
    }
    if (start < n) {  // ultima linea sin terminador explicito
        lines.emplace_back(lineno, src.substr(start));
    }
    return lines;
}

Scanner::Scanner(CodeStr source, bool strict)
    : source_(std::move(source)), strict_(strict), indents_{0} {}

// -- utilidades internas ----------------------------------------------

void Scanner::error(const std::string& msg, int line, int col) {
    if (strict_) throw LexError(msg, line, col);
    errors_.push_back(LexErrorInfo{msg, line, col});
    std::cout << "ERROR SCAN - " << msg << " (" << line << ":" << col << ")" << std::endl;
}

void Scanner::emit(Token t, const CodeStr& lexeme) {
    std::cout << "DEBUG SCAN - " << t.type
              << " [ " << utf8Encode(lexeme) << " ]"
              << " (" << t.line << ":" << t.col << ")"
              << std::endl;

    tokens_.push_back(std::move(t));
}

// -- API publica -------------------------------------------------------

const std::vector<Token>& Scanner::tokenize() {
    std::cout << "INFO SCAN - Start scanning..." << "\n";
    int lastLineno = 0;

    for (const auto& entry : physicalLines(source_)) {
        const int lineno = entry.first;
        const CodeStr& text = entry.second;
        lastLineno = lineno;
        const size_t n = text.size();

        // Indentacion: tabs expandidos a multiplos de 8 (Reference 3.1.5).
        size_t i = 0;
        int indent = 0;
        while (i < n && (text[i] == U' ' || text[i] == U'\t')) {
            indent += (text[i] == U' ') ? 1 : (8 - (indent % 8));
            i += 1;
        }

        // Linea en blanco o solo comentario (Reference 3.1.3-3.1.4).
        if (i >= n || text[i] == U'#') continue;

        // Es una linea logica (Reference 3.1.2).
        handleIndent(indent, lineno);
        scanLine(lineno, text, i);
        emit(Token::plain("NEWLINE", lineno, static_cast<int>(n) + 1),U"\\n");
    }

    // Fin de la entrada: un DEDENT por cada nivel > 0 (Reference 3.1.5).
    const int eofLine = lastLineno + 1;
    while (indents_.size() > 1) {
        indents_.pop_back();
        emit(Token::plain("DEDENT", eofLine, 1), U"<dedent>");
    }
    emit(Token::plain("EOF", eofLine, 1), U"<eof>");
    return tokens_;
}

// -- estructura de lineas ---------------------------------------------

void Scanner::handleIndent(int indent, int lineno) {
    const int top = indents_.back();
    if (indent == top) return;
    if (indent > top) {
        indents_.push_back(indent);
        emit(Token::plain("INDENT", lineno, 1), U"<indent>");
        return;
    }
    while (indents_.size() > 1 && indents_.back() > indent) {
        indents_.pop_back();
        emit(Token::plain("DEDENT", lineno, 1), U"<dedent>");
    }
    if (indents_.back() != indent) {
        error("nivel de indentacion inconsistente", lineno, 1);
        indents_.push_back(indent);  // recuperacion
    }
}

// -- reconocimiento de tokens dentro de una linea ---------------------

void Scanner::scanLine(int lineno, const CodeStr& text, size_t i) {
    const size_t n = text.size();
    while (i < n) {
        const char32_t c = text[i];
        const int col = static_cast<int>(i) + 1;

        if (c == U' ' || c == U'\t') {
            i += 1;
            continue;
        }
        if (c == U'#') return;  // comentario hasta fin de linea
        if (c == U'"') {
            i = scanString(lineno, text, i);
            continue;
        }
        if (isDigit(c)) {
            i = scanNumber(lineno, text, i);
            continue;
        }
        if (isIdentStart(c)) {
            size_t j = i + 1;
            while (j < n && isIdentPart(text[j])) j += 1;
            const std::string word = utf8Encode(text.substr(i, j - i));
            CodeStr lexeme = text.substr(i, j - i);
            if (isKeyword(word)) {
                emit(Token::plain(asciiUpper(word), lineno, col), lexeme);
            } else {
                emit(Token::text("ID", lexeme, lineno, col), lexeme);
            }
            i = j;
            continue;
        }
        i = scanOperator(lineno, text, i);
    }
}

size_t Scanner::scanOperator(int lineno, const CodeStr& text, size_t i) {
    const int col = static_cast<int>(i) + 1;

    const CodeStr two = text.substr(i, 2);
    const auto it2 = op2().find(two);
    if (two.size() == 2 && it2 != op2().end()) {
        emit(Token::plain(it2->second, lineno, col), two);
        return i + 2;
    }

    const char32_t one = text[i];
    const auto it1 = op1().find(one);
    if (it1 != op1().end()) {
        emit(Token::plain(it1->second, lineno, col), CodeStr(1, one));
        return i + 1;
    }
    if (one == U'/') {
        error("operador '/' no soportado en ChocoPy (usa '//')", lineno, col);
        return i + 1;
    }
    if (one == U'!') {
        error("'!' inesperado (se esperaba '!=')", lineno, col);
        return i + 1;
    }
    error("caracter ilegal " + pyRepr(CodeStr(1, one)), lineno, col);
    return i + 1;
}

size_t Scanner::scanNumber(int lineno, const CodeStr& text, size_t i) {
    const size_t n = text.size();
    size_t j = i;
    while (j < n && isDigit(text[j])) j += 1;

    const CodeStr lexemeCp = text.substr(i, j - i);
    const std::string lexeme = utf8Encode(lexemeCp);
    const int col = static_cast<int>(i) + 1;

    // Maximal munch: se toma la corrida completa de digitos y luego
    // se valida (Reference 3.4.2). El test de longitud va primero para
    // no convertir cadenas de digitos gigantescas.
    long long value = 0;
    bool inRange = false;
    if (lexeme.size() <= 10) {
        value = std::stoll(lexeme);
        inRange = value <= INT_MAX_CHOCOPY;
    }

    if (lexeme.size() > 1 && lexeme[0] == '0') {
        error("literal entero con cero a la izquierda: " + lexeme, lineno, col);
    } else if (!inRange) {
        error("literal entero fuera de rango (maximo " +
                  std::to_string(INT_MAX_CHOCOPY) + "): " + lexeme,
              lineno, col);
    }

    // Valor de recuperacion: el entero si es representable, si no INT_MAX.
    emit(Token::integer("INTEGER", inRange ? value : INT_MAX_CHOCOPY, lineno,
                        col), lexemeCp);
    return j;
}

size_t Scanner::scanString(int lineno, const CodeStr& text, size_t i) {
    const size_t n = text.size();
    const int col = static_cast<int>(i) + 1;
    size_t j = i + 1;
    CodeStr out;
    bool hadError = false;

    while (j < n) {
        const char32_t ch = text[j];
        if (ch == U'"') {
            if (!hadError && isIdentifier(out)) {
                emit(Token::text("IDSTRING", out, lineno, col),text.substr(i, j - i + 1));
            } else {
                emit(Token::text("STRING", out, lineno, col), text.substr(i, j - i + 1));
            }
            return j + 1;
        }
        if (ch == U'\\') {
            if (j + 1 < n) {
                const char32_t esc = text[j + 1];
                const auto it = escapes().find(esc);
                if (it != escapes().end()) {
                    out += it->second;
                } else {
                    error("secuencia de escape ilegal '\\" +
                              utf8Encode(esc) + "'",
                          lineno, static_cast<int>(j) + 1);
                    hadError = true;
                    out += esc;
                }
                j += 2;
                continue;
            }
            break;  // '\' como ultimo caracter: cadena sin terminar
        }
        const unsigned long o = static_cast<unsigned long>(ch);
        if (o < 32 || o > 126) {
            error("caracter fuera del rango ASCII 32-126 en string (codigo " +
                      std::to_string(o) + ")",
                  lineno, static_cast<int>(j) + 1);
            hadError = true;
            j += 1;
            continue;
        }
        out += ch;
        j += 1;
    }

    error("string literal sin terminar", lineno, col);
    emit(Token::text("STRING", out, lineno, col), text.substr(i, n - i));
    return n;
}
