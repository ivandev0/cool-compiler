#include "Lexer.h"
#include "Utils.h"

bool Lexer::hasNext() {
    return !isAtEnd();
}

Token Lexer::next() {
    while (offset < program.length()) {
        auto c = advance();
        switch (c) {
            case ' ':
            case '\f':
            case '\r':
            case '\t':
            case '\v':
                // Ignore whitespace.
                break;
            case '\n': lineNumber++; break;

            case '{':
            case '}':
            case ')':
            case ';':
            case '+':
            case '*':
            case '/':
            case '.':
            case '@':
            case '~':
                return {std::string(1, c), lineNumber};

            case '=':
                if (match('>')) {
                    return {Token::Kind::DARROW, lineNumber};
                } else {
                    return {std::string(1, c), lineNumber};
                }
            case '-':
                if (match('-')) {
                    // skip comment
                    while (peek() != '\n' && !isAtEnd()) advance();
                    break;
                } else {
                    return {std::string(1, c), lineNumber};
                }

            case '(':
                if (match('*')) {
                    if (!tryToSkipMultiLineComment()) return {Token::Kind::ERROR, "EOF in comment", lineNumber};
                    break;
                } else {
                    return {std::string(1, c), lineNumber};
                }

            case '<':
                if (match('-')) {
                    return {Token::Kind::ASSIGN, lineNumber};
                } else if (match('=')) {
                    return {Token::Kind::LE, lineNumber};
                } else {
                    return {std::string(1, c), lineNumber};
                }
            case '"': return string();
            default:
                if (isDigit(c)) {
                    return number();
                } else if (isAlpha(c)) {
                    return identifier();
                } else {
                    return {Token::Kind::ERROR, std::string(1, c), lineNumber};
                }
        }
    }

    throw std::runtime_error("There are no tokens left");
}

char Lexer::advance() {
    return program.at(offset++);
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return program.at(offset);
}

char Lexer::peekNext() {
    if (offset + 1 >= program.length()) return '\0';
    return program.at(offset + 1);
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (program.at(offset) != expected) return false;

    offset++;
    return true;
}

bool Lexer::isAtEnd() {
    return offset >= program.length();
}

Token Lexer::string() {
    // TODO drop '/' symbol
    auto begin = offset;
    while (peek() != '"' && !isAtEnd()) advance();
    if (isAtEnd()) return {Token::Kind::ERROR, "EOF in string constant", lineNumber};

    advance(); // skip enclosing '"'
    return {Token::Kind::STR_CONST, program.substr(begin, (offset - 1) - begin), lineNumber};
}

Token Lexer::number() {
    auto begin = offset - 1;
    while (isDigit(peek()) && !isAtEnd()) advance();
    return {Token::Kind::INT_CONST, program.substr(begin, offset - begin), lineNumber};
}

Token Lexer::identifier() {
    auto begin = offset - 1;
    while ((isAlpha(peek()) || isDigit(peek())) && !isAtEnd()) advance();

    auto text = program.substr(begin, offset - begin);
    // BOOL_CONST | OBJECTID | TYPEID | keyword
    Token::Kind type = getKeywordType(text);
    if (type == Token::Kind::BOOL_CONST) {
        return {islower(text[0]) ? Token::Kind::BOOL_CONST : Token::Kind::OBJECTID, islower(text[0]) ? toLowerCase(text) : text, lineNumber};
    } else if (type != Token::Kind::ERROR) {
        return {type, lineNumber};
    }

    return {islower(text[0]) ? Token::Kind::OBJECTID : Token::Kind::TYPEID, text, lineNumber};
}

Token::Kind Lexer::getKeywordType(const std::string& str) {
    auto lowercaseStr = toLowerCase(str);
    if (lowercaseStr == "class") return Token::Kind::CLASS;
    if (lowercaseStr == "else") return Token::Kind::ELSE;
    if (lowercaseStr == "fi") return Token::Kind::FI;
    if (lowercaseStr == "if") return Token::Kind::IF;
    if (lowercaseStr == "in") return Token::Kind::IN;
    if (lowercaseStr == "inherits") return Token::Kind::INHERITS;
    if (lowercaseStr == "let") return Token::Kind::LET;
    if (lowercaseStr == "loop") return Token::Kind::LOOP;
    if (lowercaseStr == "pool") return Token::Kind::POOL;
    if (lowercaseStr == "then") return Token::Kind::THEN;
    if (lowercaseStr == "while") return Token::Kind::WHILE;
    if (lowercaseStr == "case") return Token::Kind::CASE;
    if (lowercaseStr == "esac") return Token::Kind::ESAC;
    if (lowercaseStr == "of") return Token::Kind::OF;
    if (lowercaseStr == "new") return Token::Kind::NEW;
    if (lowercaseStr == "isvoid") return Token::Kind::ISVOID;
    if (lowercaseStr == "not") return Token::Kind::NOT;

    if (lowercaseStr == "true" || lowercaseStr == "false") {
        if (islower(str[0])) {
            return Token::Kind::BOOL_CONST;
        }
    }
    return Token::Kind::ERROR;
}

bool Lexer::tryToSkipMultiLineComment() {
    std::size_t comments = 1;
    while (comments != 0 && !isAtEnd()) {
        if (peek() == '*' && peekNext() == ')') comments--;
        if (peek() == '(' && peekNext() == '*') comments++;
        advance();
    }
    if (isAtEnd()) {
        return false;
    }
    return true;
}
