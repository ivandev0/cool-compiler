#pragma once
#include <sstream>
#include <utility>
#include "Token.h"

class Lexer {
public:
    explicit Lexer(std::string  program) : program(std::move(program)) {}

    bool hasNext();
    Token next();

private:
    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    bool isAtEnd();

    Token string();
    Token number();
    Token identifier();
    static Token::Kind getKeywordType(const std::string& str);
    bool tryToSkipMultiLineComment();
private:
    std::string program;
    std::size_t lineNumber = 1;
    std::size_t offset = 0;
    std::size_t comments = 0;
};
