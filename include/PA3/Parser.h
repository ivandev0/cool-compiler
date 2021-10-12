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

        LetStatementExpression parseLetStatementExpression();
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




//        StaticDispatchExpression parseStaticDispatchExpression();
//        DispatchExpression parseDispatchExpression();
//        IsVoidExpression parseIsVoidExpression();
//        PlusExpression parsePlusExpression();
//        MinusExpression parseMinusExpression();
//        MulExpression parseMulExpression();
//        DivExpression parseDivExpression();
//        InverseExpression parseInverseExpression();
//        LessExpression parseLessExpression();
//        LessOrEqualExpression parseLessOrEqualExpression();
//        EqualExpression parseEqualExpression();
//        NotExpression parseNotExpression();

    private:
        bool hasNext() { return iterator != end; }
        Token getNext() { return *(iterator++); }

        Token peek() { return *iterator; }
        Token peekNext() { return *(iterator + 1); /* TODO throw error if distance > 1 */ }

        bool checkNext(const std::string &symbol, const std::string &message) {
            if (peek().lexeme == symbol) return true;
            if (!message.empty()) throw std::runtime_error(message);
            return false;
        }

        bool checkNextKind(const Token::Kind &kind, const std::string &message) {
            if (peek().kind == kind) return true;
            if (!message.empty()) throw std::runtime_error(message);
            return false;
        }

        bool matchNext(const std::string &symbol, const std::string &message) {
            if (!checkNext(symbol, message)) return false;
            getNext();
            return true;
        }

        bool matchNext(const std::string &symbol) {
            return matchNext(symbol, "");
        }

        bool matchNextKind(const Token::Kind &kind, const std::string &message) {
            if (!checkNextKind(kind, message)) return false;
            getNext();
            return true;
        }

        bool matchNextKind(const Token::Kind &kind) {
            return matchNextKind(kind, "");
        }


    private:
        std::vector<Token>::const_iterator iterator;
        std::vector<Token>::const_iterator end;
        std::string filename;
    };
}
