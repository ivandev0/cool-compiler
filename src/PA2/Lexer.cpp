#include "Lexer.h"
#include "Utils.h"

bool Lexer::hasNext() {
    if (isAtEnd()) return false;
    while (!isAtEnd()) {
        if (peek() == '-' && peekNext() == '-') {
            while (peek() != '\n' && !isAtEnd()) advance();
        }
        if (peek() == '(' && peekNext() == '*') {
            advance();
            if (!tryToSkipMultiLineComment()) return true;
        }
        if (isAtEnd()) return false;
        if (!isWhitespace(peek())) return true;
        advance();
    }
    return !isAtEnd();
}

Token Lexer::next() {
    if (isAtEnd() && comments != 0) {
        return {Token::Kind::ERROR, "EOF in comment", lineNumber};
    }

    while (!isAtEnd()) {
        auto c = advance();
        if (isWhitespace(c)) continue;
        switch (c) {
            case '{':
            case '}':
            case '(':
            case ')':
            case ':':
            case ';':
            case '+':
            case '-':
            case '/':
            case '.':
            case ',':
            case '@':
            case '~':
                return {std::string(1, c), lineNumber};

            case '=':
                if (match('>')) {
                    return {Token::Kind::DARROW, lineNumber};
                } else {
                    return {std::string(1, c), lineNumber};
                }
            case '*':
                if (match(')')) {
                    return {Token::Kind::ERROR, "Unmatched *)", lineNumber};
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
                    return {Token::Kind::ERROR, charToStringRepresentation(c), lineNumber};
                }
        }
    }

    throw std::runtime_error("There are no tokens left");
}

char Lexer::advance() {
    if (program.at(offset) == '\n') lineNumber++;
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

    advance();
    return true;
}

bool Lexer::isAtEnd() {
    return offset >= program.length();
}

Token Lexer::string() {
    std::size_t size = 0;
    std::stringstream result;
    result << '"';
    char c;
    while ((c = peek()) != '"' && !isAtEnd()) {
        if (c == '\\') {
            auto next = peekNext();
            if (next == 'b' || next == 't' || next == 'n' || next == 'f' || next == '\\' || next == '"' || next == '\n') {
                next == '\n' ? result << "\\n" : result << '\\' << next;
                size++;
                advance();
            }
            if (next == '\0') {
                advance();
                return {Token::Kind::ERROR, "String contains escaped null character.", lineNumber};
            }
        } else if (c == '\n') {
            advance();
            return {Token::Kind::ERROR, "Unterminated string constant", lineNumber};
        } else if (c == '\0') {
            while (peek() != '"' && peek() != '\n' && !isAtEnd()) advance();
            if (peek() == '"') advance();
            return {Token::Kind::ERROR, "String contains null character.", lineNumber};
        } else {
            result << charToStringRepresentation(c);
            size++;
        }
        advance();
    }
    if (isAtEnd()) return {Token::Kind::ERROR, "EOF in string constant", lineNumber};

    advance(); // skip enclosing '"'
    result << '"';
    if (size > MAX_STR_LENGTH) return {Token::Kind::ERROR, "String constant too long", lineNumber};
    return {Token::Kind::STR_CONST, result.str(), lineNumber};
}

Token Lexer::number() {
    auto begin = offset - 1;
    while (isDigit(peek()) && !isAtEnd()) advance();
    return {Token::Kind::INT_CONST, program.substr(begin, offset - begin), lineNumber};
}

Token Lexer::identifier() {
    auto begin = offset - 1;
    while (isAlphaOdDigitOrUnderscore(peek()) && !isAtEnd()) advance();

    auto text = program.substr(begin, offset - begin);
    // BOOL_CONST | OBJECTID | TYPEID | keyword
    Token::Kind type = getKeywordType(text);
    if (type == Token::Kind::BOOL_CONST) {
        return {islower(text[0]) ? Token::Kind::BOOL_CONST : Token::Kind::TYPEID, islower(text[0]) ? toLowerCase(text) : text, lineNumber};
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
    if (lowercaseStr == "true" || lowercaseStr == "false") return Token::Kind::BOOL_CONST;
    return Token::Kind::ERROR;
}

bool Lexer::tryToSkipMultiLineComment() {
    comments = 1;
    while (comments != 0 && !isAtEnd()) {
        if (peek() == '*' && peekNext() == ')') comments--;
        if (peek() == '(' && peekNext() == '*') comments++;
        advance();
    }
    if (isAtEnd()) {
        return false;
    }
    advance(); // skip last ')'
    return true;
}
