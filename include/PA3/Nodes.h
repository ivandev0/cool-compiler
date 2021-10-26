#pragma once

#include <utility>
#include <variant>
#include <vector>
#include <string>

namespace parser {
    struct TreeNode {
        std::size_t line_number = 0;
        std::string result_type = "_no_type";
    };

    struct Class;
    struct Feature;
    struct AttrFeature;
    struct MethodFeature;
    struct Expression;
    struct IdExpression;

    struct Program : TreeNode {
        std::vector<Class> classes;
    };

    struct Class : TreeNode {
        std::string type;
        std::string parent;
        std::vector<Feature> features;
        std::string filename;
    };

    struct IdExpression : TreeNode {
        std::string id;
    };

    struct Formal : TreeNode {
        IdExpression id;
        std::string type;
    };

    struct AttrFeature : TreeNode {
        IdExpression id;
        std::string type;
        std::shared_ptr<Expression> expr;
    };

    struct MethodFeature : TreeNode {
        IdExpression id;
        std::vector<Formal> params;
        std::string return_type;
        std::shared_ptr<Expression> expr;
    };

    struct Feature : TreeNode {
        std::variant<AttrFeature, MethodFeature> feature;
    };


    struct AssignExpression : TreeNode {
        std::shared_ptr<IdExpression> id;
        std::shared_ptr<Expression> expr;
    };

    struct StaticDispatchExpression : TreeNode {
        std::shared_ptr<Expression> expr;
        std::string type;
        std::shared_ptr<IdExpression> id;
        std::vector<std::shared_ptr<Expression>> list;
    };

    struct DispatchExpression : TreeNode {
        std::shared_ptr<Expression> expr;
        std::shared_ptr<IdExpression> id;
        std::vector<std::shared_ptr<Expression>> list;
    };

    struct IfExpression : TreeNode {
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Expression> trueBranch;
        std::shared_ptr<Expression> falseBranch;
    };

    struct WhileExpression : TreeNode {
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Expression> body;
    };

    struct BlockExpression : TreeNode {
        std::vector<std::shared_ptr<Expression>> list;
    };

    struct LetExpression : TreeNode {
        std::shared_ptr<IdExpression> id;
        std::string type;
        std::shared_ptr<Expression> expr;
        std::shared_ptr<Expression> body;
    };

    struct CaseBranchExpression : TreeNode {
        std::shared_ptr<IdExpression> id;
        std::string type;
        std::shared_ptr<Expression> expr;
    };

    struct CaseExpression : TreeNode {
        std::shared_ptr<Expression> expr;
        std::vector<CaseBranchExpression> branches;
    };

    struct NewExpression : TreeNode {
        std::string type;
    };

    struct IsVoidExpression : TreeNode {
        std::shared_ptr<Expression> expr;
    };

    struct PlusExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct MinusExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct MulExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct DivExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct InverseExpression : TreeNode {
        std::shared_ptr<Expression> expr;
    };

    struct LessExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct LessOrEqualExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct EqualExpression : TreeNode {
        std::shared_ptr<Expression> lhs;
        std::shared_ptr<Expression> rhs;
    };

    struct NotExpression : TreeNode {
        std::shared_ptr<Expression> expr;
    };

    struct InBracketsExpression : TreeNode {
        std::shared_ptr<Expression> expr;
    };

    struct IntExpression : TreeNode {
        std::size_t value;
    };

    struct StringExpression : TreeNode {
        std::string value;
    };

    struct BoolExpression : TreeNode {
        bool value;
    };

    struct NoExprExpression : TreeNode {
        std::string value = "_no_expr";
    };

    struct Expression : TreeNode {
        std::variant<
                AssignExpression, StaticDispatchExpression, DispatchExpression, IfExpression,
                WhileExpression, BlockExpression, LetExpression, CaseExpression, NewExpression, IsVoidExpression,
                PlusExpression, MinusExpression, MulExpression, DivExpression, InverseExpression,
                LessExpression, LessOrEqualExpression, EqualExpression, NotExpression, InBracketsExpression,
                IdExpression, IntExpression, StringExpression, BoolExpression, NoExprExpression
        > data;
    };
}