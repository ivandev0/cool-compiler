#pragma once
#include <vector>
#include <string>

#include "Token.h"

namespace parser {
    struct Program {
    public:
        std::string toString() {
            return "NOT IMPLEMENTED\n";
        }
    };

    class Parser {
    public:
        explicit Parser(const std::vector<Token> &tokens) : iterator(tokens.cbegin()) {}
        Program parseProgram();

    private:
        std::vector<Token>::const_iterator iterator;
    };
}