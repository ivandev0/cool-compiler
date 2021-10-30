#pragma once
#include <sstream>
#include <utility>
#include "Token.h"

namespace lexer {
    class Lexer {
    public:
        explicit Lexer(std::istream& program, std::string file_name) : program_(program), file_name_(std::move(file_name)) {}

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
        std::istream& program_;
        std::string file_name_;
        std::size_t line_number_ = -1;
        std::size_t comments_ = 0;

        static const std::size_t MAX_STR_LENGTH_ = 1024;
    };

}