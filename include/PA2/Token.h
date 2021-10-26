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

    Token(Kind kind, std::size_t line) : kind_(kind), lexeme_(""), line_(line) {}

    Token(std::string lexeme, std::size_t line) : kind_(Kind::ATOM), lexeme_(std::move(lexeme)), line_(line) {}

    Token(Kind kind, std::string lexeme, std::size_t line) : kind_(kind), lexeme_(std::move(lexeme)), line_(line) {}

    std::string ToString() const;
    std::string ToStringForParser() const;

public:
    Kind kind_;
    std::string lexeme_;
    std::size_t line_;
};
