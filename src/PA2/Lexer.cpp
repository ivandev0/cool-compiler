#include "Lexer.h"
#include "Utils.h"

bool lexer::Lexer::HasNext() {
    if (line_number_ == -1) return true;
    if (IsAtEnd()) return false;
    while (!IsAtEnd()) {
        if (Peek() == '-' && PeekNext() == '-') {
            while (Peek() != '\n' && !IsAtEnd()) Advance();
        }
        if (Peek() == '(' && PeekNext() == '*') {
            Advance();
            if (!TryToSkipMultiLineComment()) return true;
        }
        if (IsAtEnd()) return false;
        if (!IsWhitespace(Peek())) return true;
        Advance();
    }
    return !IsAtEnd();
}

Token lexer::Lexer::Next() {
    if (line_number_ == -1) {
        line_number_ = 1;
        return Token{file_name_, 0};
    }

    if (IsAtEnd() && comments_ != 0) {
        return {Token::Kind::ERROR, "EOF in comment", line_number_};
    }

    while (!IsAtEnd()) {
        auto c = Peek();
        if (IsWhitespace(c)) { Advance(); continue; }
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
                Advance();
                return {std::string(1, c), line_number_};

            case '=':
                Advance();
                if (Match('>')) {
                    return {Token::Kind::DARROW, line_number_};
                } else {
                    return {std::string(1, c), line_number_};
                }
            case '*':
                Advance();
                if (Match(')')) {
                    return {Token::Kind::ERROR, "Unmatched *)", line_number_};
                } else {
                    return {std::string(1, c), line_number_};
                }

            case '<':
                Advance();
                if (Match('-')) {
                    return {Token::Kind::ASSIGN, line_number_};
                } else if (Match('=')) {
                    return {Token::Kind::LE, line_number_};
                } else {
                    return {std::string(1, c), line_number_};
                }
            case '"': return String();
            default:
                if (IsDigit(c)) {
                    return Number();
                } else if (IsAlpha(c)) {
                    return Identifier();
                } else {
                    Advance();
                    return {Token::Kind::ERROR, CharToStringRepresentation(c), line_number_};
                }
        }
    }

    throw std::runtime_error("There are no tokens left");
}

char lexer::Lexer::Advance() {
    if (program_.peek() == '\n') line_number_++;
    return (char) program_.get();
}

char lexer::Lexer::Peek() {
    if (IsAtEnd()) return '\0';
    return (char) program_.peek();
}

char lexer::Lexer::PeekNext() {
    if (IsAtEnd()) return '\0';
    program_.get();
    if (IsAtEnd()) return '\0';
    char c = (char) program_.peek();
    program_.unget();
    return c;
}

bool lexer::Lexer::Match(char expected) {
    if (IsAtEnd()) return false;
    if (program_.peek() != expected) return false;

    Advance();
    return true;
}

bool lexer::Lexer::IsAtEnd() {
    program_.peek(); // extra peek to set eof flag (?)
    return program_.eof();
}

Token lexer::Lexer::String() {
    std::size_t size = 0;
    std::stringstream result;
    result << Advance(); // get '"'
    char c;
    while ((c = Peek()) != '"' && !IsAtEnd()) {
        if (c == '\\') {
            auto next = PeekNext();
            if (next == 'b' || next == 't' || next == 'n' || next == 'f' || next == '\\' || next == '"' || next == '\n') {
                next == '\n' ? result << "\\n" : result << '\\' << next;
                size++;
                Advance();
            }
            if (next == '\0') {
                Advance();
                return {Token::Kind::ERROR, "String contains escaped null character.", line_number_};
            }
        } else if (c == '\n') {
            Advance();
            return {Token::Kind::ERROR, "Unterminated string constant", line_number_};
        } else if (c == '\0') {
            while (Peek() != '"' && Peek() != '\n' && !IsAtEnd()) Advance();
            if (Peek() == '"') Advance();
            return {Token::Kind::ERROR, "String contains null character.", line_number_};
        } else {
            result << CharToStringRepresentation(c);
            size++;
        }
        Advance();
    }
    if (IsAtEnd()) return {Token::Kind::ERROR, "EOF in string constant", line_number_};

    Advance(); // skip enclosing '"'
    result << '"';
    if (size > MAX_STR_LENGTH_) return {Token::Kind::ERROR, "String constant too long", line_number_};
    return {Token::Kind::STR_CONST, result.str(), line_number_};
}

Token lexer::Lexer::Number() {
    std::vector<char> number;
    while (IsDigit(Peek()) && !IsAtEnd()) number.push_back(Advance());
    std::string number_str(number.begin(), number.end());
    return {Token::Kind::INT_CONST, number_str, line_number_};
}

Token lexer::Lexer::Identifier() {
    std::vector<char> id;
    while (IsAlphaOdDigitOrUnderscore(Peek()) && !IsAtEnd()) id.push_back(Advance());

    std::string text(id.begin(), id.end());
    // BOOL_CONST | OBJECTID | TYPEID | keyword
    Token::Kind type = GetKeywordType(text);
    if (type == Token::Kind::BOOL_CONST) {
        return {islower(text[0]) ? Token::Kind::BOOL_CONST : Token::Kind::TYPEID, islower(text[0]) ? ToLowerCase(text) : text, line_number_};
    } else if (type != Token::Kind::ERROR) {
        return {type, line_number_};
    }

    return {islower(text[0]) ? Token::Kind::OBJECTID : Token::Kind::TYPEID, text, line_number_};
}

Token::Kind lexer::Lexer::GetKeywordType(const std::string& str) {
    auto lowercase_str = ToLowerCase(str);
    if (lowercase_str == "class") return Token::Kind::CLASS;
    if (lowercase_str == "else") return Token::Kind::ELSE;
    if (lowercase_str == "fi") return Token::Kind::FI;
    if (lowercase_str == "if") return Token::Kind::IF;
    if (lowercase_str == "in") return Token::Kind::IN;
    if (lowercase_str == "inherits") return Token::Kind::INHERITS;
    if (lowercase_str == "let") return Token::Kind::LET;
    if (lowercase_str == "loop") return Token::Kind::LOOP;
    if (lowercase_str == "pool") return Token::Kind::POOL;
    if (lowercase_str == "then") return Token::Kind::THEN;
    if (lowercase_str == "while") return Token::Kind::WHILE;
    if (lowercase_str == "case") return Token::Kind::CASE;
    if (lowercase_str == "esac") return Token::Kind::ESAC;
    if (lowercase_str == "of") return Token::Kind::OF;
    if (lowercase_str == "new") return Token::Kind::NEW;
    if (lowercase_str == "isvoid") return Token::Kind::ISVOID;
    if (lowercase_str == "not") return Token::Kind::NOT;
    if (lowercase_str == "true" || lowercase_str == "false") return Token::Kind::BOOL_CONST;
    return Token::Kind::ERROR;
}

bool lexer::Lexer::TryToSkipMultiLineComment() {
    comments_ = 1;
    while (comments_ != 0 && !IsAtEnd()) {
        if (Peek() == '*' && PeekNext() == ')') comments_--;
        if (Peek() == '(' && PeekNext() == '*') comments_++;
        Advance();
    }
    if (IsAtEnd()) {
        return false;
    }
    Advance(); // skip last ')'
    return true;
}
