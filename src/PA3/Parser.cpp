#include <vector>
#include "Parser.h"

namespace parser {
    Program Parser::parseProgram() {
        if (!hasNext()) throwError();
        std::size_t line = iterator->line;
        std::vector<Class> classes;
        while (hasNext()) {
            classes.push_back(parseClass());
            matchNext(";");
        }
        return Program{{line}, classes};
    }

    Class Parser::parseClass() {
        std::size_t line = iterator->line;

        matchNextKind(Token::CLASS);

        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;
        std::string parent;
        if (matchNextKind(Token::INHERITS, false)) {
            checkNextKind(Token::TYPEID);
            parent = getNext().lexeme;
        } else {
            parent = "Object";
        }

        matchNext("{");
        std::vector<Feature> features;
        while (peek().lexeme != "}") {
            features.push_back(parseFeature());
        }
        matchNext("}");

        return Class{{line}, type, parent, features, filename};
    }

    Feature Parser::parseFeature() {
        std::size_t line = iterator->line;
        auto feature = peekNext().lexeme == ":" ?
                Feature{{line}, parseAttrFeature()} :
                Feature{{line}, parseMethodFeature()};
        matchNext(";");
        return feature;
    }

    AttrFeature Parser::parseAttrFeature() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID);

        auto id = parseIdExpression();
        matchNext(":");
        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;
        auto expr = std::make_shared<Expression>(matchNextKind(Token::ASSIGN, false) ? parseExpression() : createNoExpr());

        return AttrFeature{{line}, id, type, expr};
    }

    MethodFeature Parser::parseMethodFeature() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID);

        auto id = parseIdExpression();
        matchNext("(");
        std::vector<Formal> formals;
        while (peek().lexeme != ")") {
            formals.push_back(parseFormal());
            if (peek().lexeme != ")") matchNext(",");
        }
        matchNext(")");
        matchNext(":");
        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;

        matchNext("{");
        auto expr = std::make_shared<Expression>(parseExpression());
        matchNext("}");

        return MethodFeature{{line}, id, formals, type, expr};
    }

    Formal Parser::parseFormal() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID);
        auto id = parseIdExpression();
        matchNext(":");
        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;
        return Formal{{line}, id, type};
    }

//  expr ::= ID <- expr
//      | not_expr
//
//  not_expr ::= not not_expr
//      | comp_expr
//
//  comp_expr ::= plus_sub_expr ( ( "<=" | "<" | "=") plus_sub_expr )*
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
//  atom ::= ID <- expr
//      | if expr then expr else expr fi
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

    Expression Parser::parseExpression() {
        std::size_t line = iterator->line;
        if (peek().kind == Token::OBJECTID && peekNext().kind == Token::ASSIGN) {
            auto id = std::make_shared<IdExpression>(parseIdExpression());
            iterator++; //skip assign
            return Expression{{line}, AssignExpression{{line}, id, std::make_shared<Expression>(parseExpression())}};
        }
        return parseNotExpression();
    }

    Expression Parser::parseNotExpression() {
        std::size_t line = iterator->line;

        if (matchNextKind(Token::NOT, false)) {
            return Expression{{line}, NotExpression{{line}, std::make_shared<Expression>(parseNotExpression())}};
        }
        return parseComparisonExpression();
    }

    Expression Parser::parseComparisonExpression() {
        std::size_t line = iterator->line;
        auto term = parsePlusSubExpression();

        auto next = peek();
        while (next.kind == Token::LE || next.lexeme == "<" || next.lexeme == "=") {
            iterator++;
            auto next_term = std::make_shared<Expression>(parsePlusSubExpression());
            if (next.kind == Token::LE) {
                term = Expression{{line}, LessOrEqualExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            } else if (next.lexeme == "<") {
                term = Expression{{line}, LessExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            } else {
                term = Expression{{line}, EqualExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            }
            next = peek();
            line = iterator->line;
        }

        return term;
    }

    Expression Parser::parsePlusSubExpression() {
        std::size_t line = iterator->line;
        auto term = parseMulDivExpression();

        auto next = peek();
        while (next.lexeme == "+" || next.lexeme == "-") {
            iterator++;
            auto next_term = std::make_shared<Expression>(parseMulDivExpression());
            if (next.lexeme == "+") {
                term = Expression{{line}, PlusExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            } else {
                term = Expression{{line}, MinusExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            }
            next = peek();
            line = iterator->line;
        }

        return term;
    }

    Expression Parser::parseMulDivExpression() {
        std::size_t line = iterator->line;
        auto term = parseIsVoidExpression();

        auto next = peek();
        while (next.lexeme == "*" || next.lexeme == "/") {
            iterator++;
            auto next_term = std::make_shared<Expression>(parseIsVoidExpression());
            if (next.lexeme == "*") {
                term = Expression{{line}, MulExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            } else {
                term = Expression{{line}, DivExpression{{line}, std::make_shared<Expression>(std::move(term)), next_term}};
            }
            next = peek();
            line = iterator->line;
        }

        return term;
    }

    Expression Parser::parseIsVoidExpression() {
        std::size_t line = iterator->line;
        if (matchNextKind(Token::ISVOID, false)) {
            return Expression{{line}, IsVoidExpression{{line}, std::make_shared<Expression>(parseIsVoidExpression())}};
        }
        return parseNegExpression();
    }

    Expression Parser::parseNegExpression() {
        std::size_t line = iterator->line;
        if (matchNext("~", false)) {
            return Expression{{line}, InverseExpression{{line}, std::make_shared<Expression>(parseNegExpression())}};
        }
        return parseStaticDispatchExpression();
    }

    Expression Parser::parseStaticDispatchExpression() {
        auto term = parseDispatchExpression();
        auto next = peek();
        while (next.lexeme == "@" || next.lexeme == ".") {
            std::size_t line = iterator->line;

            std::string type;
            if (matchNext("@", false)) {
                checkNextKind(Token::TYPEID);
                type = getNext().lexeme;
            } else {
                type = "";
            }

            matchNext(".");
            checkNextKind(Token::OBJECTID);
            auto id = std::make_shared<IdExpression>(parseIdExpression());

            matchNext("(");
            std::vector<std::shared_ptr<Expression>> args;
            while (peek().lexeme != ")") {
                args.push_back(std::make_shared<Expression>(parseExpression()));
                if (peek().lexeme != ")") matchNext(",");
            }
            matchNext(")");

            if (type.empty()) {
                term = Expression{{line}, DispatchExpression{{line}, std::make_shared<Expression>(term), id, args}};
            } else {
                term = Expression{{line}, StaticDispatchExpression{{line}, std::make_shared<Expression>(term), type, id, args}};
            }
            next = peek();
        }

        return term;
    }

    Expression Parser::parseDispatchExpression() {
        std::size_t line = iterator->line;

        if (peek().kind != Token::OBJECTID || peekNext().lexeme != "(") return parseAtomExpression();
        Expression expr = Expression{{line}, IdExpression{{line}, "self"}};
        checkNextKind(Token::OBJECTID);
        auto id = parseIdExpression();

        matchNext("(");
        std::vector<std::shared_ptr<Expression>> args;
        while (peek().lexeme != ")") {
            if (!args.empty()) matchNext(",");
            args.push_back(std::make_shared<Expression>(parseExpression()));
        }
        matchNext(")");

        return Expression{
                {line},
                DispatchExpression{
                        {line}, std::make_shared<Expression>(expr), std::make_shared<IdExpression>(id), args
                }
        };
    }

    Expression Parser::parseAtomExpression() {
        std::size_t line = iterator->line;
        auto next = peek();
        if (next.kind == Token::OBJECTID && peekNext().kind == Token::ASSIGN) {
            return Expression{{line}, parseAssignExpression()};
        } else if (next.kind == Token::IF) {
            return Expression{{line}, parseIfExpression()};
        } else if (next.kind == Token::WHILE) {
            return Expression{{line}, parseWhileExpression()};
        } else if (next.lexeme == "{") {
            return Expression{{line}, parseBlockExpression()};
        } else if (next.kind == Token::LET) {
            return Expression{{line}, parseLetExpression()};
        } else if (next.kind == Token::CASE) {
            return Expression{{line}, parseCaseExpression()};
        } else if (next.kind == Token::NEW) {
            return Expression{{line}, parseNewExpression()};
        } else if (next.kind == Token::OBJECTID) {
            return Expression{{line}, parseIdExpression()};
        } else if (next.lexeme == "(") {
            return Expression{{line}, parseInBracketsExpression()};
        } else if (next.kind == Token::INT_CONST) {
            return Expression{{line}, parseIntExpression()};
        } else if (next.kind == Token::STR_CONST) {
            return Expression{{line}, parseStringExpression()};
        } else if (next.kind == Token::BOOL_CONST) {
            return Expression{{line}, parseBoolExpression()};
        }

        throwError();
        return createNoExpr(); // Unreachable code
    }

    AssignExpression Parser::parseAssignExpression() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID);
        auto id = std::make_shared<IdExpression>(parseIdExpression());
        matchNextKind(Token::ASSIGN);
        auto expr = std::make_shared<Expression>(parseExpression());
        return AssignExpression{{line}, id, expr};
    }

    IfExpression Parser::parseIfExpression() {
        std::size_t line = iterator->line;
        matchNextKind(Token::IF);
        auto condition = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::THEN);
        auto trueBranch = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::ELSE);
        auto falseBranch = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::FI);
        return IfExpression{{line}, condition, trueBranch, falseBranch};
    }

    WhileExpression Parser::parseWhileExpression() {
        std::size_t line = iterator->line;
        matchNextKind(Token::WHILE);
        auto condition = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::LOOP);
        auto body = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::POOL);
        return WhileExpression{{line}, condition, body};
    }

    BlockExpression Parser::parseBlockExpression() {
        std::size_t line = iterator->line;
        matchNext("{");
        std::vector<std::shared_ptr<Expression>> exprs;
        while (peek().lexeme != "}") {
            exprs.push_back(std::make_shared<Expression>(parseExpression()));
            matchNext(";", "");
        }
        if (exprs.empty()) throwError();
        matchNext("}");
        return BlockExpression{{line}, exprs};
    }

    LetExpression Parser::parseLetExpression() {
        matchNextKind(Token::LET);
        checkNextKind(Token::OBJECTID);
        return parseInnerLetExpression();
    }

    CaseExpression Parser::parseCaseExpression() {
        std::size_t line = iterator->line;
        matchNextKind(Token::CASE);
        auto condition = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::OF);

        std::vector<CaseBranchExpression> branches;
        while (peek().kind != Token::ESAC) {
            branches.push_back(parseCaseBranchExpression());
        }
        matchNextKind(Token::ESAC);
        return CaseExpression{{line}, condition, branches};
    }

    NewExpression Parser::parseNewExpression() {
        std::size_t line = iterator->line;
        matchNextKind(Token::NEW);
        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;
        return NewExpression{{line}, type};
    }

    InBracketsExpression Parser::parseInBracketsExpression() {
        std::size_t line = iterator->line;
        matchNext("(");
        auto expr = std::make_shared<Expression>(parseExpression());
        matchNext(")");
        return InBracketsExpression{{line}, expr};
    }

    IntExpression Parser::parseIntExpression() {
        std::size_t line = iterator->line;
        const char* p = (iterator++)->lexeme.c_str();
        char *endOfP;
        return IntExpression{{line}, std::strtoul(p, &endOfP, 10)};
    }

    StringExpression Parser::parseStringExpression() {
        std::size_t line = iterator->line;
        return StringExpression{{line}, (iterator++)->lexeme};
    }

    BoolExpression Parser::parseBoolExpression() {
        std::size_t line = iterator->line;
        return BoolExpression{{line}, (iterator++)->lexeme == "true"};
    }

    IdExpression Parser::parseIdExpression() {
        std::size_t line = iterator->line;
        return IdExpression{{line}, getNext().lexeme};
    }

    LetExpression Parser::parseInnerLetExpression() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID);
        auto id = std::make_shared<IdExpression>(parseIdExpression());
        matchNext(":");
        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;
        auto expr = std::make_shared<Expression>(matchNextKind(Token::ASSIGN, false) ? parseExpression() : createNoExpr());
        if (peek().kind != Token::IN) matchNext(",");
        return LetExpression{
            {line}, id, type, expr,
            std::make_shared<Expression>(matchNextKind(Token::IN, false) ? parseExpression() : Expression{{line}, parseInnerLetExpression()})
        };
    }

    CaseBranchExpression Parser::parseCaseBranchExpression() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID);
        auto id = std::make_shared<IdExpression>(parseIdExpression());
        matchNext(":");
        checkNextKind(Token::TYPEID);
        auto type = getNext().lexeme;
        matchNextKind(Token::DARROW);
        auto expr = std::make_shared<Expression>(parseExpression());
        matchNext(";");
        return CaseBranchExpression{{line}, id, type, expr};
    }
}
