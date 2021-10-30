#pragma once
#include <sstream>
#include <utility>
#include "Token.h"

namespace lexer {
    struct LexerInput {
        std::istream& program;
        std::string file_name;
    };

    class Lexer {
    public:
        virtual bool HasNext() = 0;
        virtual Token Next() = 0;
    };

    class SingleFileLexer: Lexer {
    public:
        explicit SingleFileLexer(LexerInput input) : program_(input.program), file_name_(std::move(input.file_name)) {}

        bool HasNext() override;
        Token Next() override;

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

    class MultiFileLexer: Lexer {
    public:
        explicit MultiFileLexer(std::vector<LexerInput> inputs): inputs_(std::move(inputs)) {}

        bool HasNext() override {
            if (lexer_ == nullptr && current_ >= inputs_.size()) return false;
            if (lexer_ == nullptr) lexer_ = new SingleFileLexer(inputs_[current_++]);
            if (lexer_->HasNext()) return true;

            delete lexer_;
            lexer_ = nullptr;
            return HasNext();
        }

        Token Next() override {
            return lexer_->Next();
        }

        virtual ~MultiFileLexer() {
            delete lexer_;
            lexer_ = nullptr;
        }
    private:
        std::vector<LexerInput> inputs_;
        std::size_t current_ = 0;
        SingleFileLexer* lexer_ = nullptr;
    };
}