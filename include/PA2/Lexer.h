#pragma once
#include <sstream>
#include <utility>
#include "Token.h"

namespace lexer {
    class Lexer {
    public:
        explicit Lexer(std::string  program) : program_(std::move(program)) {}

        bool HasNext();
        Token Next();

    private:
        char Advance();
        char Peek();
        char PeekNext();
        bool Match(char expected);
        bool IsAtEnd();

        Token String();
        Token Number();
        Token Identifier();
        static Token::Kind GetKeywordType(const std::string& str);
        bool TryToSkipMultiLineComment();
    private:
        std::string program_;
        std::size_t line_number_ = 1;
        std::size_t offset_ = 0;
        std::size_t comments_ = 0;

        static const std::size_t MAX_STR_LENGTH_ = 1024;
    };

}