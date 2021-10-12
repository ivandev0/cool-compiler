#include <vector>
#include "Parser.h"

namespace parser {
    Program Parser::parseProgram() {
        std::size_t line = iterator->line;
        std::vector<Class> classes;
        while (hasNext()) {
            classes.push_back(parseClass());
            matchNext(";", "Error"); // TODO better message
        }
        return Program{{line}, classes};
    }

    Class Parser::parseClass() {
        std::size_t line = iterator->line;

        matchNextKind(Token::CLASS, "Error"); // TODO better message

        checkNextKind(Token::TYPEID, "Error"); // TODO better message
        auto type = getNext().lexeme;
        std::string parent = matchNextKind(Token::INHERITS) ? getNext().lexeme : "Object";

        matchNext("{", "Error");  // TODO better message
        std::vector<Feature> features;
        while (peek().lexeme != "}") {
            features.push_back(parseFeature());
        }
        matchNext("}", "Error");  // TODO better message

        return Class{{line}, type, parent, features, filename}; // TODO filename
    }

    Feature Parser::parseFeature() {
        std::size_t line = iterator->line;
        auto feature = peekNext().lexeme == ":" ?
                Feature{{line}, parseAttrFeature()} :
                Feature{{line}, parseMethodFeature()};
        matchNext(";", "Error");
        return feature;
    }

    AttrFeature Parser::parseAttrFeature() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message

        auto id = parseIdExpression();
        matchNext(":", "Error");  // TODO better message
        checkNextKind(Token::TYPEID, "Error"); // TODO better message
        auto type = getNext().lexeme;
        auto expr = matchNextKind(Token::ASSIGN) ? std::make_shared<Expression>(parseExpression()) : nullptr;

        return AttrFeature{{line}, id, type, expr};
    }

    MethodFeature Parser::parseMethodFeature() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message

        auto id = parseIdExpression();
        matchNext("(", "Error");  // TODO better message
        std::vector<Formal> formals;
        while (peek().lexeme != ")") {
            formals.push_back(parseFormal());
        }
        matchNext(")", "Error");  // TODO better message
        matchNext(":", "Error");  // TODO better message
        checkNextKind(Token::TYPEID, "Error"); // TODO better message
        auto type = getNext().lexeme;

        matchNext("{", "Error");  // TODO better message
        auto expr = std::make_shared<Expression>(parseExpression());
        matchNext("}", "Error");  // TODO better message

        return MethodFeature{{line}, id, formals, type, expr};
    }

    Formal Parser::parseFormal() {
        std::size_t line = iterator->line;
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message
        auto id = parseIdExpression();
        matchNext(":", "Error");  // TODO better message
        checkNextKind(Token::TYPEID, "Error"); // TODO better message
        auto type = getNext().lexeme;
        return Formal{{line}, id, type};
    }

//  expr ::= ID <- expr
//      | not_expr
//
//  not_expr ::= not expr
//      | comp_expr
//
//  comp_expr ::= plus_sub_expr ( ( "<=" | "<" | "=") expr )*
//  plus_sub_expr ::= mul_div_expr ( ( "+" | "-") expr )*
//  mul_div_expr ::= isvoid_expr ( ( "*" | "/") expr )*
//
//  isvoid_expr ::= isvoid expr
//      | neg_expr
//
//  neg_expr ::= ~ expr
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

        if (matchNextKind(Token::NOT)) {
            return Expression{{line}, NotExpression{{line}, std::make_shared<Expression>(parseExpression())}};
        }
        return parseComparisonExpression();
    }

    Expression Parser::parseComparisonExpression() {
        std::size_t line = iterator->line;
        auto term = parsePlusSubExpression();

        auto next = peek();
        while (next.kind == Token::LE || next.lexeme == "<" || next.lexeme == "=") {
            iterator++;
            auto next_term = std::make_shared<Expression>(parseExpression());
            if (next.kind == Token::LE) {
                term = Expression{{line}, LessOrEqualExpression{{line}, next_term}};
            } else if (next.lexeme == "<") {
                term = Expression{{line}, LessExpression{{line}, next_term}};
            } else {
                term = Expression{{line}, EqualExpression{{line}, next_term}};
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
            auto next_term = std::make_shared<Expression>(parseExpression());
            if (next.lexeme == "+") {
                term = Expression{{line}, PlusExpression{{line}, next_term}};
            } else {
                term = Expression{{line}, MinusExpression{{line}, next_term}};
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
            auto next_term = std::make_shared<Expression>(parseExpression());
            if (next.lexeme == "*") {
                term = Expression{{line}, MulExpression{{line}, next_term}};
            } else {
                term = Expression{{line}, DivExpression{{line}, next_term}};
            }
            next = peek();
            line = iterator->line;
        }

        return term;
    }

    Expression Parser::parseIsVoidExpression() {
        std::size_t line = iterator->line;
        if (matchNextKind(Token::ISVOID)) {
            return Expression{{line}, IsVoidExpression{{line}, std::make_shared<Expression>(parseExpression())}};
        }
        return parseNegExpression();
    }

    Expression Parser::parseNegExpression() {
        std::size_t line = iterator->line;
        if (matchNext("~")) {
            return Expression{{line}, InverseExpression{{line}, std::make_shared<Expression>(parseExpression())}};
        }
        return parseStaticDispatchExpression();
    }

    Expression Parser::parseStaticDispatchExpression() {
        auto term = parseDispatchExpression();
        auto next = peek();
        while (next.lexeme == "@" || next.lexeme == ".") {
            std::size_t line = iterator->line;

            std::string type;
            if (matchNext("@")) {
                checkNextKind(Token::TYPEID, "Error"); // TODO better message
                type = getNext().lexeme;
            } else {
                type = "";
            }

            matchNext(".", "Error"); // TODO better message
            checkNextKind(Token::OBJECTID, "Error"); // TODO better message
            auto id = std::make_shared<IdExpression>(parseIdExpression());

            matchNext("(", "Error"); // TODO better message
            std::vector<std::shared_ptr<Expression>> args;
            while (peek().lexeme != ")") { // TODO and not eof
                args.push_back(std::make_shared<Expression>(parseExpression()));
            }
            matchNext(")", "Error"); // TODO better message

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
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message
        auto id = parseIdExpression();

        matchNext("(", "Error"); // TODO better message
        std::vector<std::shared_ptr<Expression>> args;
        while (peek().lexeme != ")") { // TODO and not eof
            args.push_back(std::make_shared<Expression>(parseExpression()));
        }
        matchNext(")", "Error"); // TODO better message

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

        throw std::runtime_error("Unsupported expression");
    }

    AssignExpression Parser::parseAssignExpression() {
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message
        auto id = std::make_shared<IdExpression>(parseIdExpression());
        matchNextKind(Token::ASSIGN, "Error");
        auto expr = std::make_shared<Expression>(parseExpression());
        return AssignExpression{{iterator->line}, id, expr};
    }

    IfExpression Parser::parseIfExpression() {
        matchNextKind(Token::IF, "Error");
        auto condition = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::THEN, "Error");
        auto trueBranch = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::ELSE, "Error");
        auto falseBranch = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::FI, "Error");
        return IfExpression{{iterator->line}, condition, trueBranch, falseBranch};
    }

    WhileExpression Parser::parseWhileExpression() {
        matchNextKind(Token::WHILE, "Error");
        auto condition = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::LOOP, "Error");
        auto body = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::POOL, "Error");
        return WhileExpression{{iterator->line}, condition, body};
    }

    BlockExpression Parser::parseBlockExpression() {
        matchNext("{", "Error");
        std::vector<std::shared_ptr<Expression>> exprs;
        while (peek().lexeme != "}") { // TODO and not eof
            exprs.push_back(std::make_shared<Expression>(parseExpression()));
            matchNext(";", "");
        }
        matchNext("}", "Error");
        return BlockExpression{{iterator->line}, exprs};
    }

    LetExpression Parser::parseLetExpression() {
        matchNextKind(Token::LET, "Error");
        checkNextKind(Token::OBJECTID, "Error");

        std::vector<LetStatementExpression> stmts;
        while (peek().kind != Token::IN) { // TODO and not eof
            stmts.push_back(parseLetStatementExpression());
        }
        matchNextKind(Token::IN, "Error"); // TODO better message
        auto body = std::make_shared<Expression>(parseExpression());
        return LetExpression{{iterator->line}, stmts, body};
    }

    CaseExpression Parser::parseCaseExpression() {
        matchNextKind(Token::CASE, "Error");
        auto condition = std::make_shared<Expression>(parseExpression());
        matchNextKind(Token::OF, "Error");

        std::vector<CaseBranchExpression> branches;
        while (peek().kind != Token::ESAC) { // TODO and not eof
            branches.push_back(parseCaseBranchExpression());
        }
        matchNextKind(Token::ESAC, "Error");
        return CaseExpression{{iterator->line}, condition, branches};
    }

    NewExpression Parser::parseNewExpression() {
        matchNextKind(Token::NEW, "Error");
        checkNextKind(Token::TYPEID, "Error");
        auto type = getNext().lexeme;
        return NewExpression{{iterator->line}, type};
    }

    InBracketsExpression Parser::parseInBracketsExpression() {
        matchNext("(", "Error"); // TODO better message
        auto expr = std::make_shared<Expression>(parseExpression());
        matchNext(")", "Error"); // TODO better message
        return InBracketsExpression{{iterator->line}, expr};
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
        return BoolExpression{{line}, (iterator++)->lexeme == "True"};
    }

    IdExpression Parser::parseIdExpression() {
        std::size_t line = iterator->line;
        return IdExpression{{line}, getNext().lexeme};
    }

    LetStatementExpression Parser::parseLetStatementExpression() {
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message
        auto id = std::make_shared<IdExpression>(parseIdExpression());
        matchNext(":", "Error");  // TODO better message
        checkNextKind(Token::TYPEID, "Error"); // TODO better message
        auto type = getNext().lexeme;
        matchNextKind(Token::ASSIGN, "Error"); // TODO better message
        auto expr = std::make_shared<Expression>(parseExpression());
        return LetStatementExpression{{iterator->line}, id, type, expr};
    }

    CaseBranchExpression Parser::parseCaseBranchExpression() {
        checkNextKind(Token::OBJECTID, "Error"); // TODO better message
        auto id = std::make_shared<IdExpression>(parseIdExpression());
        matchNext(":", "Error");  // TODO better message
        checkNextKind(Token::TYPEID, "Error"); // TODO better message
        auto type = getNext().lexeme;
        matchNextKind(Token::DARROW, "Error"); // TODO better message
        auto expr = std::make_shared<Expression>(parseExpression());
        matchNext(";", "Error"); // TODO better message
        return CaseBranchExpression{{iterator->line}, id, type, expr};
    }
}
