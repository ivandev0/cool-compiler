#pragma once
#include <utility>
#include <vector>
#include <string>

#include "Token.h"
#include "Nodes.h"

namespace parser {
    class Parser {
    public:
        Parser(const std::vector<Token> &tokens, std::string filename)
            : iterator_(tokens.cbegin()), end_(tokens.cend()), filename_(std::move(filename)) {}
        Program ParseProgram();

    private:
        Class ParseClass();
        Feature ParseFeature();
        AttrFeature ParseAttrFeature();
        MethodFeature ParseMethodFeature();
        Formal ParseFormal();

        Expression ParseExpression();
        IfExpression ParseIfExpression();
        WhileExpression ParseWhileExpression();
        BlockExpression ParseBlockExpression();
        LetExpression ParseLetExpression();
        CaseExpression ParseCaseExpression();
        NewExpression ParseNewExpression();
        InBracketsExpression ParseInBracketsExpression();
        IntExpression ParseIntExpression();
        StringExpression ParseStringExpression();
        BoolExpression ParseBoolExpression();
        IdExpression ParseIdExpression();

        LetExpression ParseInnerLetExpression();
        CaseBranchExpression ParseCaseBranchExpression();

        Expression ParseNotExpression();
        Expression ParseComparisonExpression();
        Expression ParsePlusSubExpression();
        Expression ParseMulDivExpression();
        Expression ParseIsVoidExpression();
        Expression ParseNegExpression();
        Expression ParseStaticDispatchExpression();
        Expression ParseDispatchExpression();
        Expression ParseAtomExpression();

    private:
        bool HasNext() { return iterator_ != end_; }
        Token GetNext() { return *(iterator_++); }

        Token Peek() {
            if (HasNext()) return *iterator_;
            return {"EOF", 0};
        }
        Token PeekNext() {
            if (!HasNext() || (iterator_ + 1) == end_) return {"EOF", 0};
            return *(iterator_ + 1);
        }

        void ThrowError() {
            auto tokenStr = !HasNext() ? "EOF" : iterator_->ToStringForParser();
            auto line = !HasNext() ? "0" : std::to_string(iterator_->line_);
            auto message = "\"" + filename_ + "\", line " + line + ": syntax error at or near " + tokenStr;
            throw std::runtime_error(message);
        }

        bool CheckNext(const std::string &symbol, bool with_error = true) {
            if (Peek().lexeme_ == symbol) return true;
            if (with_error) ThrowError();
            return false;
        }

        bool CheckNextKind(const Token::Kind &kind, bool with_error = true) {
            if (Peek().kind_ == kind) return true;
            if (with_error) ThrowError();
            return false;
        }

        bool MatchNext(const std::string &symbol, bool with_error = true) {
            if (!CheckNext(symbol, with_error)) return false;
            GetNext();
            return true;
        }

        bool MatchNextKind(const Token::Kind &kind, bool with_error = true) {
            if (!CheckNextKind(kind, with_error)) return false;
            GetNext();
            return true;
        }

        static Expression CreateNoExpr() {
            return {{0}, NoExprExpression{}};
        }

    private:
        std::vector<Token>::const_iterator iterator_;
        std::vector<Token>::const_iterator end_;
        std::string filename_;
    };
}
