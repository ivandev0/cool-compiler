#include <vector>
#include "Parser.h"

namespace parser {
    Program Parser::ParseProgram() {
        if (!HasNext()) ThrowError();
        filename_ = GetNext().lexeme_;

        if (!HasNext()) ThrowError();
        std::size_t line = iterator_->line_;
        std::vector<Class> classes;
        while (HasNext()) {
            classes.push_back(ParseClass());
            MatchNext(";");
        }
        return Program{{line}, classes};
    }

    Class Parser::ParseClass() {
        if (Peek().line_ == 0) filename_ = GetNext().lexeme_;
        std::size_t line = iterator_->line_;

        MatchNextKind(Token::CLASS);
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;
        std::string parent;
        if (MatchNextKind(Token::INHERITS, false)) {
            CheckNextKind(Token::TYPEID);
            parent = GetNext().lexeme_;
        } else {
            parent = "Object";
        }

        MatchNext("{");
        std::vector<Feature> features;
        while (Peek().lexeme_ != "}") {
            features.push_back(ParseFeature());
        }
        MatchNext("}");

        return Class{{line}, type, parent, features, filename_};
    }

    Feature Parser::ParseFeature() {
        std::size_t line = iterator_->line_;
        auto feature = PeekNext().lexeme_ == ":" ?
                       Feature{{line}, ParseAttrFeature()} :
                       Feature{{line}, ParseMethodFeature()};
        MatchNext(";");
        return feature;
    }

    AttrFeature Parser::ParseAttrFeature() {
        std::size_t line = iterator_->line_;
        CheckNextKind(Token::OBJECTID);

        auto id = ParseIdExpression();
        MatchNext(":");
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;
        auto expr = MatchNextKind(Token::ASSIGN, false) ? ParseExpression() : CreateNoExpr();

        return AttrFeature{{line}, id, type, std::make_shared<Expression>(expr)};
    }

    MethodFeature Parser::ParseMethodFeature() {
        std::size_t line = iterator_->line_;
        CheckNextKind(Token::OBJECTID);

        auto id = ParseIdExpression();
        MatchNext("(");
        std::vector<Formal> formals;
        while (Peek().lexeme_ != ")") {
            formals.push_back(ParseFormal());
            if (Peek().lexeme_ != ")") MatchNext(",");
        }
        MatchNext(")");
        MatchNext(":");
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;

        MatchNext("{");
        auto expr = ParseExpression();
        MatchNext("}");

        return MethodFeature{{line}, id, formals, type, std::make_shared<Expression>(expr)};
    }

    Formal Parser::ParseFormal() {
        std::size_t line = iterator_->line_;
        CheckNextKind(Token::OBJECTID);
        auto id = ParseIdExpression();
        MatchNext(":");
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;
        return Formal{{line}, id, type};
    }

//  expr ::= ID <- expr
//      | not_expr
//
//  not_expr ::= not not_expr
//      | comp_expr
//
//  comp_expr ::= plus_sub_expr ( "<=" | "<" | "=") plus_sub_expr
//  plus_sub_expr ::= mul_div_expr ( ( "+" | "-") mul_div_expr )*
//  mul_div_expr ::= isvoid_expr ( ( "*" | "/") isvoid_expr )*
//
//  isvoid_expr ::= isvoid isvoid_expr
//      | neg_expr
//
//  neg_expr ::= ~ neg_expr
//      | static_dispatch_expr
//
//  static_dispatch_expr ::= dispatch_expr([@TYPE]?.ID( [ expr [[, expr]] ∗ ] ))*
//
//  dispatch_expr ::= ID( [ expr [[, expr]] ∗ ] )
//      | atom
//
//  atom ::= if expr then expr else expr fi
//      | while expr loop expr pool
//      | { [[expr; ]] + }
//      | let ID : TYPE [ <- expr ] [[, ID : TYPE [ <- expr ]]] ∗ in expr
//      | case expr of [[ID : TYPE => expr; ]] + esac
//      | new TYPE(expr)
//      | (expr)
//      | ID
//      | integer
//      | string
//      | true
//      | false

    Expression Parser::ParseExpression() {
        std::size_t line = iterator_->line_;
        if (Peek().kind_ == Token::OBJECTID && PeekNext().kind_ == Token::ASSIGN) {
            auto id = ParseIdExpression();
            iterator_++; //skip assign
            auto expr = ParseExpression();
            return Expression{{line}, AssignExpression{{line}, std::make_shared<IdExpression>(id), std::make_shared<Expression>(expr)}};
        }
        return ParseNotExpression();
    }

    Expression Parser::ParseNotExpression() {
        std::size_t line = iterator_->line_;

        if (MatchNextKind(Token::NOT, false)) {
            return Expression{{line}, NotExpression{{line}, std::make_shared<Expression>(ParseNotExpression())}};
        }
        return ParseComparisonExpression();
    }

    Expression Parser::ParseComparisonExpression() {
        auto term = ParsePlusSubExpression();
        std::size_t line = iterator_->line_;

        auto next = Peek();
        if (next.kind_ == Token::LE || next.lexeme_ == "<" || next.lexeme_ == "=") {
            iterator_++;
            auto next_term = ParsePlusSubExpression();
            if (next.kind_ == Token::LE) {
                term = Expression{
                    {line},
                    LessOrEqualExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            } else if (next.lexeme_ == "<") {
                term = Expression{
                    {line},
                    LessExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            } else {
                term = Expression{
                    {line},
                    EqualExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            }
        }

        return term;
    }

    Expression Parser::ParsePlusSubExpression() {
        auto term = ParseMulDivExpression();
        std::size_t line = iterator_->line_;

        auto next = Peek();
        while (next.lexeme_ == "+" || next.lexeme_ == "-") {
            iterator_++;
            auto next_term = ParseMulDivExpression();
            if (next.lexeme_ == "+") {
                term = Expression{
                    {line},
                    PlusExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            } else {
                term = Expression{
                    {line},
                    MinusExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            }
            next = Peek();
            line = iterator_->line_;
        }

        return term;
    }

    Expression Parser::ParseMulDivExpression() {
        auto term = ParseIsVoidExpression();
        std::size_t line = iterator_->line_;

        auto next = Peek();
        while (next.lexeme_ == "*" || next.lexeme_ == "/") {
            iterator_++;
            auto next_term = ParseIsVoidExpression();
            if (next.lexeme_ == "*") {
                term = Expression{
                    {line},
                    MulExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            } else {
                term = Expression{
                    {line},
                    DivExpression{{line}, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(next_term)}
                };
            }
            next = Peek();
            line = iterator_->line_;
        }

        return term;
    }

    Expression Parser::ParseIsVoidExpression() {
        std::size_t line = iterator_->line_;
        if (MatchNextKind(Token::ISVOID, false)) {
            return Expression{{line}, IsVoidExpression{{line}, std::make_shared<Expression>(ParseIsVoidExpression())}};
        }
        return ParseNegExpression();
    }

    Expression Parser::ParseNegExpression() {
        std::size_t line = iterator_->line_;
        if (MatchNext("~", false)) {
            return Expression{{line}, InverseExpression{{line}, std::make_shared<Expression>(ParseNegExpression())}};
        }
        return ParseStaticDispatchExpression();
    }

    Expression Parser::ParseStaticDispatchExpression() {
        auto term = ParseDispatchExpression();
        auto next = Peek();
        while (next.lexeme_ == "@" || next.lexeme_ == ".") {
            std::size_t line = iterator_->line_;

            std::string type;
            if (MatchNext("@", false)) {
                CheckNextKind(Token::TYPEID);
                type = GetNext().lexeme_;
            } else {
                type = "";
            }

            MatchNext(".");
            CheckNextKind(Token::OBJECTID);
            auto id = ParseIdExpression();

            MatchNext("(");
            std::vector<std::shared_ptr<Expression>> args;
            while (Peek().lexeme_ != ")") {
                args.push_back(std::make_shared<Expression>(ParseExpression()));
                if (Peek().lexeme_ != ")") MatchNext(",");
            }
            MatchNext(")");

            if (type.empty()) {
                term = Expression{
                    {line},
                    DispatchExpression{{line}, std::make_shared<Expression>(term), std::make_shared<IdExpression>(id), args}
                };
            } else {
                term = Expression{
                    {line},
                    StaticDispatchExpression{{line}, std::make_shared<Expression>(term), type, std::make_shared<IdExpression>(id), args}
                };
            }
            next = Peek();
        }

        return term;
    }

    Expression Parser::ParseDispatchExpression() {
        std::size_t line = iterator_->line_;

        if (Peek().kind_ != Token::OBJECTID || PeekNext().lexeme_ != "(") return ParseAtomExpression();
        Expression expr = Expression{{line}, IdExpression{{line}, "self"}};
        CheckNextKind(Token::OBJECTID);
        auto id = ParseIdExpression();

        MatchNext("(");
        std::vector<std::shared_ptr<Expression>> args;
        while (Peek().lexeme_ != ")") {
            if (!args.empty()) MatchNext(",");
            args.push_back(std::make_shared<Expression>(ParseExpression()));
        }
        MatchNext(")");

        return Expression{
                {line},
                DispatchExpression{
                    {line}, std::make_shared<Expression>(expr), std::make_shared<IdExpression>(id), args
                }
        };
    }

    Expression Parser::ParseAtomExpression() {
        std::size_t line = iterator_->line_;
        auto next = Peek();
        if (next.kind_ == Token::IF) {
            return Expression{{line}, ParseIfExpression()};
        } else if (next.kind_ == Token::WHILE) {
            return Expression{{line}, ParseWhileExpression()};
        } else if (next.lexeme_ == "{") {
            return Expression{{line}, ParseBlockExpression()};
        } else if (next.kind_ == Token::LET) {
            return Expression{{line}, ParseLetExpression()};
        } else if (next.kind_ == Token::CASE) {
            return Expression{{line}, ParseCaseExpression()};
        } else if (next.kind_ == Token::NEW) {
            return Expression{{line}, ParseNewExpression()};
        } else if (next.kind_ == Token::OBJECTID) {
            return Expression{{line}, ParseIdExpression()};
        } else if (next.lexeme_ == "(") {
            return Expression{{line}, ParseInBracketsExpression()};
        } else if (next.kind_ == Token::INT_CONST) {
            return Expression{{line}, ParseIntExpression()};
        } else if (next.kind_ == Token::STR_CONST) {
            return Expression{{line}, ParseStringExpression()};
        } else if (next.kind_ == Token::BOOL_CONST) {
            return Expression{{line}, ParseBoolExpression()};
        }

        ThrowError();
        return CreateNoExpr(); // Unreachable code
    }

    IfExpression Parser::ParseIfExpression() {
        std::size_t line = iterator_->line_;
        MatchNextKind(Token::IF);
        auto condition = ParseExpression();
        MatchNextKind(Token::THEN);
        auto trueBranch = ParseExpression();
        MatchNextKind(Token::ELSE);
        auto falseBranch = ParseExpression();
        MatchNextKind(Token::FI);
        return IfExpression{
            {line},
            std::make_shared<Expression>(condition),
            std::make_shared<Expression>(trueBranch),
            std::make_shared<Expression>(falseBranch)
        };
    }

    WhileExpression Parser::ParseWhileExpression() {
        std::size_t line = iterator_->line_;
        MatchNextKind(Token::WHILE);
        auto condition = ParseExpression();
        MatchNextKind(Token::LOOP);
        auto body = ParseExpression();
        MatchNextKind(Token::POOL);
        return WhileExpression{{line}, std::make_shared<Expression>(condition), std::make_shared<Expression>(body)};
    }

    BlockExpression Parser::ParseBlockExpression() {
        std::size_t line = iterator_->line_;
        MatchNext("{");
        std::vector<std::shared_ptr<Expression>> exprs;
        while (Peek().lexeme_ != "}") {
            exprs.push_back(std::make_shared<Expression>(ParseExpression()));
            MatchNext(";", "");
        }
        if (exprs.empty()) ThrowError();
        MatchNext("}");
        return BlockExpression{{line}, exprs};
    }

    LetExpression Parser::ParseLetExpression() {
        MatchNextKind(Token::LET);
        CheckNextKind(Token::OBJECTID);
        return ParseInnerLetExpression();
    }

    CaseExpression Parser::ParseCaseExpression() {
        std::size_t line = iterator_->line_;
        MatchNextKind(Token::CASE);
        auto condition = ParseExpression();
        MatchNextKind(Token::OF);

        std::vector<CaseBranchExpression> branches;
        while (Peek().kind_ != Token::ESAC) {
            branches.push_back(ParseCaseBranchExpression());
        }
        MatchNextKind(Token::ESAC);
        return CaseExpression{{line}, std::make_shared<Expression>(condition), branches};
    }

    NewExpression Parser::ParseNewExpression() {
        std::size_t line = iterator_->line_;
        MatchNextKind(Token::NEW);
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;
        return NewExpression{{line}, type};
    }

    InBracketsExpression Parser::ParseInBracketsExpression() {
        std::size_t line = iterator_->line_;
        MatchNext("(");
        auto expr = ParseExpression();
        MatchNext(")");
        return InBracketsExpression{{line}, std::make_shared<Expression>(expr)};
    }

    IntExpression Parser::ParseIntExpression() {
        std::size_t line = iterator_->line_;
        const char* p = (iterator_++)->lexeme_.c_str();
        char *endOfP;
        return IntExpression{{line}, std::strtoul(p, &endOfP, 10)};
    }

    StringExpression Parser::ParseStringExpression() {
        std::size_t line = iterator_->line_;
        return StringExpression{{line}, (iterator_++)->lexeme_};
    }

    BoolExpression Parser::ParseBoolExpression() {
        std::size_t line = iterator_->line_;
        return BoolExpression{{line}, (iterator_++)->lexeme_ == "true"};
    }

    IdExpression Parser::ParseIdExpression() {
        std::size_t line = iterator_->line_;
        CheckNextKind(Token::OBJECTID);
        return IdExpression{{line}, GetNext().lexeme_};
    }

    LetExpression Parser::ParseInnerLetExpression() {
        std::size_t line = iterator_->line_;
        CheckNextKind(Token::OBJECTID);
        auto id = ParseIdExpression();
        MatchNext(":");
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;
        auto expr = MatchNextKind(Token::ASSIGN, false) ? ParseExpression() : CreateNoExpr();
        if (Peek().kind_ != Token::IN) MatchNext(",");
        auto body = MatchNextKind(Token::IN, false) ? ParseExpression() : Expression{{line}, ParseInnerLetExpression()};
        return LetExpression{
            {line},
            std::make_shared<IdExpression>(id),
            type,
            std::make_shared<Expression>(expr),
            std::make_shared<Expression>(body)
        };
    }

    CaseBranchExpression Parser::ParseCaseBranchExpression() {
        std::size_t line = iterator_->line_;
        CheckNextKind(Token::OBJECTID);
        auto id = ParseIdExpression();
        MatchNext(":");
        CheckNextKind(Token::TYPEID);
        auto type = GetNext().lexeme_;
        MatchNextKind(Token::DARROW);
        auto expr = ParseExpression();
        MatchNext(";");
        return CaseBranchExpression{{line}, std::make_shared<IdExpression>(id), type, std::make_shared<Expression>(expr)};
    }
}
