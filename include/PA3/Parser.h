#pragma once
#include <utility>
#include <vector>
#include <string>

#include "Token.h"
#include "Nodes.h"

namespace parser {
    class Parser {
    public:
        explicit Parser(const std::vector<Token> &tokens, std::string filename)
            : iterator(tokens.cbegin()), end(tokens.cend()), filename(std::move(filename)) {}
        Program parseProgram();

    private:
        Class parseClass();
        Feature parseFeature();
        AttrFeature parseAttrFeature();
        MethodFeature parseMethodFeature();
        Formal parseFormal();

        Expression parseExpression();
        AssignExpression parseAssignExpression();
        IfExpression parseIfExpression();
        WhileExpression parseWhileExpression();
        BlockExpression parseBlockExpression();
        LetExpression parseLetExpression();
        CaseExpression parseCaseExpression();
        NewExpression parseNewExpression();
        InBracketsExpression parseInBracketsExpression();
        IntExpression parseIntExpression();
        StringExpression parseStringExpression();
        BoolExpression parseBoolExpression();
        IdExpression parseIdExpression();

        LetExpression parseInnerLetExpression();
        CaseBranchExpression parseCaseBranchExpression();

        Expression parseNotExpression();
        Expression parseComparisonExpression();
        Expression parsePlusSubExpression();
        Expression parseMulDivExpression();
        Expression parseIsVoidExpression();
        Expression parseNegExpression();
        Expression parseStaticDispatchExpression();
        Expression parseDispatchExpression();
        Expression parseAtomExpression();

    private:
        bool hasNext() { return iterator != end; }
        Token getNext() { return *(iterator++); }

        Token peek() {
            if (hasNext()) return *iterator;
            return {"EOF", 0};
        }
        Token peekNext() {
            if (!hasNext() || (iterator + 1) == end) return {"EOF", 0};
            return *(iterator + 1);
        }

        void throwError() {
            auto tokenStr = !hasNext() ? "EOF" : iterator->toStringForParser();
            auto line = !hasNext() ? "0" : std::to_string(iterator->line);
            auto message = "\"" + filename + "\", line " + line + ": syntax error at or near " + tokenStr;
            throw std::runtime_error(message);
        }

        bool checkNext(const std::string &symbol, bool withError = true) {
            if (peek().lexeme == symbol) return true;
            if (withError) throwError();
            return false;
        }

        bool checkNextKind(const Token::Kind &kind, bool withError = true) {
            if (peek().kind == kind) return true;
            if (withError) throwError();
            return false;
        }

        bool matchNext(const std::string &symbol, bool withError = true) {
            if (!checkNext(symbol, withError)) return false;
            getNext();
            return true;
        }

        bool matchNextKind(const Token::Kind &kind, bool withError = true) {
            if (!checkNextKind(kind, withError)) return false;
            getNext();
            return true;
        }

        static Expression createNoExpr() {
            return {{0}, NoExprExpression{}};
        }

    private:
        std::vector<Token>::const_iterator iterator;
        std::vector<Token>::const_iterator end;
        std::string filename;
    };
}
