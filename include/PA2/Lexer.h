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
    std::string program;
    std::size_t lineNumber = 0;
    std::size_t offset = 0;
};
