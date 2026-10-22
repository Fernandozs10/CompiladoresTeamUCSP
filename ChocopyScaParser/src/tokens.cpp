#include "tokens.h"

#include <cstdio>
#include <unordered_set>

// --- Palabras clave (Reference 3.3) --------------------------------------
const std::vector<std::string> KEYWORDS = {
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is",
    "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
    "while", "with", "yield",
};

bool isKeyword(const std::string& word) {
    static const std::unordered_set<std::string> set(KEYWORDS.begin(),
                                                     KEYWORDS.end());
    return set.find(word) != set.end();
}

// --- Operadores y delimitadores (Reference 3.5) --------------------------
const std::vector<OperatorInfo> OPERATORS = {
    {"PLUS", "+"},        {"MINUS", "-"},      {"MULT", "*"},
    {"INTDIV", "//"},     {"MOD", "%"},        {"LT", "<"},
    {"GT", ">"},          {"LE", "<="},        {"GE", ">="},
    {"EQ", "=="},         {"NE", "!="},        {"ASSIGN", "="},
    {"LPAREN", "("},      {"RPAREN", ")"},     {"LBRACKET", "["},
    {"RBRACKET", "]"},    {"COMMA", ","},      {"COLON", ":"},
    {"DOT", "."},         {"ARROW", "->"},
};

// --- Tokens de estructura de linea (Reference 3.1) -----------------------
const std::vector<std::string> STRUCTURE = {"NEWLINE", "INDENT", "DEDENT",
                                            "EOF"};

// -------------------------------------------------------------------------
// UTF-8
// -------------------------------------------------------------------------

std::string utf8Encode(char32_t c) {
    std::string out;
    const uint32_t v = static_cast<uint32_t>(c);
    if (v < 0x80) {
        out += static_cast<char>(v);
    } else if (v < 0x800) {
        out += static_cast<char>(0xC0 | (v >> 6));
        out += static_cast<char>(0x80 | (v & 0x3F));
    } else if (v < 0x10000) {
        out += static_cast<char>(0xE0 | (v >> 12));
        out += static_cast<char>(0x80 | ((v >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (v & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (v >> 18));
        out += static_cast<char>(0x80 | ((v >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((v >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (v & 0x3F));
    }
    return out;
}

std::string utf8Encode(const CodeStr& s) {
    std::string out;
    out.reserve(s.size());
    for (char32_t c : s) out += utf8Encode(c);
    return out;
}

// U+FFFD REPLACEMENT CHARACTER: lo que Python inserta con errors="replace".
static const char32_t kReplacementChar = 0xFFFD;

CodeStr utf8Decode(const std::string& bytes) {
    CodeStr out;
    const size_t n = bytes.size();
    auto byteAt = [&](size_t k) -> uint32_t {
        return static_cast<unsigned char>(bytes[k]);
    };
    auto isCont = [&](size_t k) -> bool {
        return k < n && (byteAt(k) & 0xC0) == 0x80;
    };

    size_t i = 0;
    while (i < n) {
        const uint32_t c = byteAt(i);
        if (c < 0x80) {
            out += static_cast<char32_t>(c);
            i += 1;
        } else if ((c & 0xE0) == 0xC0 && isCont(i + 1)) {
            const uint32_t cp = ((c & 0x1Fu) << 6) | (byteAt(i + 1) & 0x3Fu);
            if (cp < 0x80) {  // sobrelargo
                out += kReplacementChar;
                i += 1;
            } else {
                out += static_cast<char32_t>(cp);
                i += 2;
            }
        } else if ((c & 0xF0) == 0xE0 && isCont(i + 1) && isCont(i + 2)) {
            const uint32_t cp = ((c & 0x0Fu) << 12) |
                                ((byteAt(i + 1) & 0x3Fu) << 6) |
                                (byteAt(i + 2) & 0x3Fu);
            if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) {
                out += kReplacementChar;
                i += 1;
            } else {
                out += static_cast<char32_t>(cp);
                i += 3;
            }
        } else if ((c & 0xF8) == 0xF0 && isCont(i + 1) && isCont(i + 2) &&
                   isCont(i + 3)) {
            const uint32_t cp = ((c & 0x07u) << 18) |
                                ((byteAt(i + 1) & 0x3Fu) << 12) |
                                ((byteAt(i + 2) & 0x3Fu) << 6) |
                                (byteAt(i + 3) & 0x3Fu);
            if (cp < 0x10000 || cp > 0x10FFFF) {
                out += kReplacementChar;
                i += 1;
            } else {
                out += static_cast<char32_t>(cp);
                i += 4;
            }
        } else {
            out += kReplacementChar;
            i += 1;
        }
    }
    return out;
}

// -------------------------------------------------------------------------
// repr() de Python y escape JSON
// -------------------------------------------------------------------------

// repr() de Python escapa todo caracter NO IMPRIMIBLE, es decir los de las
// categorias Unicode Cc, Cf, Cs, Co, Cn, Zl, Zp y Zs (salvo el espacio
// U+0020). Aqui cubrimos las categorias con rangos estables: los controles,
// los separadores, los caracteres de formato (BOM, zero-width, marcas
// bidi...), los sustitutos y el uso privado. No se cubre Cn (no asignados),
// que requeriria las tablas Unicode completas.
static bool isPyPrintable(char32_t c) {
    // Cc: controles C0 y C1.
    if (c < 0x20 || (c >= 0x7F && c <= 0x9F)) return false;
    // Zs distintos del espacio, mas Zl y Zp.
    if (c == 0x00A0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200A) ||
        c == 0x2028 || c == 0x2029 || c == 0x202F || c == 0x205F ||
        c == 0x3000) {
        return false;
    }
    // Cf: caracteres de formato.
    if (c == 0x00AD || (c >= 0x0600 && c <= 0x0605) || c == 0x061C ||
        c == 0x06DD || c == 0x070F || c == 0x180E ||
        (c >= 0x200B && c <= 0x200F) || (c >= 0x202A && c <= 0x202E) ||
        (c >= 0x2060 && c <= 0x2064) || (c >= 0x2066 && c <= 0x206F) ||
        c == 0xFEFF || (c >= 0xFFF9 && c <= 0xFFFB) || c == 0x110BD ||
        (c >= 0x1BCA0 && c <= 0x1BCA3) || (c >= 0x1D173 && c <= 0x1D17A) ||
        c == 0xE0001 || (c >= 0xE0020 && c <= 0xE007F)) {
        return false;
    }
    // Cs: sustitutos.
    if (c >= 0xD800 && c <= 0xDFFF) return false;
    // Co: uso privado.
    if ((c >= 0xE000 && c <= 0xF8FF) || (c >= 0xF0000 && c <= 0xFFFFD) ||
        (c >= 0x100000 && c <= 0x10FFFD)) {
        return false;
    }
    return true;
}

// Escape numerico al estilo de Python: \xNN, \uXXXX o \UXXXXXXXX.
static std::string pyNumericEscape(char32_t c) {
    char buf[16];
    const unsigned v = static_cast<unsigned>(c);
    if (v < 0x100) {
        std::snprintf(buf, sizeof buf, "\\x%02x", v);
    } else if (v < 0x10000) {
        std::snprintf(buf, sizeof buf, "\\u%04x", v);
    } else {
        std::snprintf(buf, sizeof buf, "\\U%08x", v);
    }
    return buf;
}

std::string pyRepr(const CodeStr& s) {
    bool hasSingle = false;
    bool hasDouble = false;
    for (char32_t c : s) {
        if (c == U'\'') hasSingle = true;
        else if (c == U'"') hasDouble = true;
    }
    const char32_t quote = (hasSingle && !hasDouble) ? U'"' : U'\'';

    std::string out;
    out += utf8Encode(quote);
    for (char32_t c : s) {
        if (c == quote || c == U'\\') {
            out += '\\';
            out += utf8Encode(c);
        } else if (c == U'\t') {
            out += "\\t";
        } else if (c == U'\n') {
            out += "\\n";
        } else if (c == U'\r') {
            out += "\\r";
        } else if (!isPyPrintable(c)) {
            out += pyNumericEscape(c);
        } else {
            out += utf8Encode(c);
        }
    }
    out += utf8Encode(quote);
    return out;
}

std::string jsonEscape(const CodeStr& s) {
    std::string out = "\"";
    for (char32_t c : s) {
        switch (c) {
            case U'"':  out += "\\\""; break;
            case U'\\': out += "\\\\"; break;
            case U'\n': out += "\\n";  break;
            case U'\t': out += "\\t";  break;
            case U'\r': out += "\\r";  break;
            case U'\b': out += "\\b";  break;
            case U'\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\u%04x",
                                  static_cast<unsigned>(c));
                    out += buf;
                } else {
                    out += utf8Encode(c);
                }
        }
    }
    out += '"';
    return out;
}

// -------------------------------------------------------------------------
// Token
// -------------------------------------------------------------------------

Token Token::plain(std::string type, int line, int col) {
    Token t;
    t.type = std::move(type);
    t.kind = ValueKind::None;
    t.line = line;
    t.col = col;
    return t;
}

Token Token::integer(std::string type, long long value, int line, int col) {
    Token t;
    t.type = std::move(type);
    t.kind = ValueKind::Int;
    t.intValue = value;
    t.line = line;
    t.col = col;
    return t;
}

Token Token::text(std::string type, CodeStr value, int line, int col) {
    Token t;
    t.type = std::move(type);
    t.kind = ValueKind::Str;
    t.strValue = std::move(value);
    t.line = line;
    t.col = col;
    return t;
}

static std::string rstrip(std::string s) {
    size_t end = s.size();
    while (end > 0) {
        const unsigned char c = static_cast<unsigned char>(s[end - 1]);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ||
            c == '\v') {
            --end;
        } else {
            break;
        }
    }
    s.resize(end);
    return s;
}

std::string Token::str() const {
    std::string shown;
    if (kind == ValueKind::None) {
        shown = "";
    } else if (type == "STRING" || type == "IDSTRING") {
        shown = pyRepr(strValue);
    } else if (kind == ValueKind::Int) {
        shown = std::to_string(intValue);
    } else {
        shown = utf8Encode(strValue);
    }
    return rstrip(std::to_string(line) + ":" + std::to_string(col) + "\t" +
                  type + "\t" + shown);
}

std::string LexErrorInfo::what() const {
    return std::to_string(line) + ":" + std::to_string(col) +
           ": error lexico: " + message;
}

std::string render(const std::vector<Token>& tokens,
                   const std::vector<LexErrorInfo>& errors) {
    std::string out;
    bool first = true;
    for (const Token& t : tokens) {
        if (!first) out += "\n";
        out += t.str();
        first = false;
    }
    for (const LexErrorInfo& e : errors) {
        if (!first) out += "\n";
        out += "ERROR\t" + std::to_string(e.line) + ":" +
               std::to_string(e.col) + "\t" + e.message;
        first = false;
    }
    return out;
}
