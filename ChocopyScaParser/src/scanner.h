
#ifndef CHOCOPY_SCANNER_H
#define CHOCOPY_SCANNER_H

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "tokens.h"

constexpr long long INT_MAX_CHOCOPY = 2147483647LL;

class LexError : public std::runtime_error
{
public:
    LexError(std::string message, int line, int col)
        : std::runtime_error(LexErrorInfo{message, line, col}.what()),
          info{std::move(message), line, col} {}

    LexErrorInfo info;
};

std::vector<std::pair<int, CodeStr>> physicalLines(const CodeStr &src);

class Scanner
{
public:
    explicit Scanner(CodeStr source, bool strict = false, bool quiet = false);

    const std::vector<Token> &tokenize();

    const std::vector<Token> &tokens() const { return tokens_; }
    const std::vector<LexErrorInfo> &errors() const { return errors_; }

private:
    void error(const std::string &msg, int line, int col);
    void emit(Token t, const CodeStr &lexeme);
    void handleIndent(int indent, int lineno, const CodeStr& text);
    void scanLine(int lineno, const CodeStr &text, size_t i);
    size_t scanOperator(int lineno, const CodeStr &text, size_t i);
    size_t scanNumber(int lineno, const CodeStr &text, size_t i);
    size_t scanString(int lineno, const CodeStr &text, size_t i);

    CodeStr source_;
    bool strict_;
    bool quiet_;
    std::vector<Token> tokens_;
    std::vector<LexErrorInfo> errors_;
    std::vector<int> indents_;
};

#endif
