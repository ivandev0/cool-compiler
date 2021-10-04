#include "Token.h"

std::string asString(Token::Kind kind) {
    switch (kind) {
        case Token::Kind::CLASS: return "CLASS";
        case Token::Kind::ELSE: return "ELSE";
        case Token::Kind::FI: return "FI";
        case Token::Kind::IF: return "IF";
        case Token::Kind::IN: return "IN";
        case Token::Kind::INHERITS: return "INHERITS";
        case Token::Kind::LET: return "LET";
        case Token::Kind::LOOP: return "LOOP";
        case Token::Kind::POOL: return "POOL";
        case Token::Kind::THEN: return "THEN";
        case Token::Kind::WHILE: return "WHILE";
        case Token::Kind::CASE: return "CASE";
        case Token::Kind::ESAC: return "ESAC";
        case Token::Kind::OF: return "OF";
        case Token::Kind::DARROW: return "DARROW";
        case Token::Kind::NEW: return "NEW";
        case Token::Kind::ISVOID: return "ISVOID";
        case Token::Kind::STR_CONST: return "STR_CONST";
        case Token::Kind::INT_CONST: return "INT_CONST";
        case Token::Kind::BOOL_CONST: return "BOOL_CONST";
        case Token::Kind::TYPEID: return "TYPEID";
        case Token::Kind::OBJECTID: return "OBJECTID";
        case Token::Kind::ASSIGN: return "ASSIGN";
        case Token::Kind::NOT: return "NOT";
        case Token::Kind::LE: return "LE";
        case Token::Kind::ERROR: return "ERROR";
        case Token::Kind::LET_STMT: return "LET_STMT";
        default: throw std::runtime_error("Unknown kind of token");
    }
}

std::string Token::toString() {
    if (kind == Kind::ATOM) {
        return "#" + std::to_string(line) + " " + lexeme;
    }
    return "#" + std::to_string(line) + " " + asString(kind) + " " + lexeme;
}
