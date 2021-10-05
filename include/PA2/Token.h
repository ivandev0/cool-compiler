#pragma once
#include <string>
#include <utility>
#include <stdexcept>
#include <map>
#include <regex>

class Token {
public:
    enum Kind {
        CLASS,
        ELSE,
        FI,
        IF,
        IN,
        INHERITS,
        LET,
        LOOP,
        POOL,
        THEN,
        WHILE,
        CASE,
        ESAC,
        OF,
        DARROW,
        NEW,
        ISVOID,
        STR_CONST,
        INT_CONST,
        BOOL_CONST,
        TYPEID,
        OBJECTID,
        ASSIGN,
        NOT,
        LE,
        LET_STMT, // TODO check that is this
        ERROR,
        ATOM // used for single atom symbol
    };

    Token(Kind kind, std::size_t line) : kind(kind), lexeme(""), line(line) {}

    Token(std::string lexeme, std::size_t line) : kind(Kind::ATOM), lexeme(std::move(lexeme)), line(line) {}

    Token(Kind kind, std::string lexeme, std::size_t line) : kind(kind), lexeme(std::move(lexeme)), line(line) {}

    std::string toString();
private:
    Kind kind;
    std::string lexeme;
    std::size_t line;
};
