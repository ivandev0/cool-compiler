#pragma once
#include <string>
#include <iomanip>
#include "ASTVisitor.h"

namespace parser {
    class PrintVisitor: public ASTVisitor<void> {
    public:
        void VisitProgram(parser::Program &program) override {
            PrintLine(program);
            PrintStr("_program");
            IncreaseOffset();
            for (auto klass: program.classes) {
                VisitClass(klass);
            }
            DecreaseOffset();
        }

        std::string GetResult() {
            return stream.str();
        }

    private:
        void IncreaseOffset() { offset += 2; }
        void DecreaseOffset() { offset -= 2; }

        void PrintLine(const TreeNode &node) {
            auto lineNumber = std::to_string(node.line_number);
            stream << std::setw(static_cast<int>(offset + 1)) << "#" << lineNumber << std::endl;
        }

        void PrintStr(const std::string &name) {
            stream << std::setw(static_cast<int>(offset + name.length())) << name << std::endl;
        }

        void PrintType(const TreeNode &node) {
            stream << std::setw(static_cast<int>(offset + 2)) << ": " << node.result_type << std::endl;
        }

        // TODO these methods can be in handy if we introduce new type - Symbol
        void PrintSymbol(const std::string &symbol) {
            if (symbol.empty()) return;
            stream << std::setw(static_cast<int>(offset + symbol.length())) << symbol << std::endl;
        }

        void VisitClass(Class &klass) override {
            PrintLine(klass);
            PrintStr("_class");
            IncreaseOffset();
            PrintSymbol(klass.type);
            PrintSymbol(klass.parent);
            PrintStr("\"" + klass.filename + "\"");
            PrintStr("(");
            for (auto feature: klass.features) {
                VisitFeature(feature);
            }
            PrintStr(")");
            DecreaseOffset();
        }

        void VisitAttrFeature(AttrFeature &attrFeature) override {
            PrintLine(attrFeature);
            PrintStr("_attr");
            IncreaseOffset();
            PrintStr(attrFeature.id.id);    // must be `PrintStr` to match original parser
            PrintSymbol(attrFeature.type);
            VisitExpression(*attrFeature.expr);
            DecreaseOffset();
        }

        void VisitMethodFeature(MethodFeature &methodFeature) override {
            PrintLine(methodFeature);
            PrintStr("_method");
            IncreaseOffset();
            PrintStr(methodFeature.id.id);    // must be `PrintStr` to match original parser
            for (auto param: methodFeature.params) {
                VisitFormal(param);
            }
            PrintSymbol(methodFeature.return_type);
            VisitExpression(*methodFeature.expr);
            DecreaseOffset();
        }

        void VisitFormal(Formal &formal) override {
            PrintLine(formal);
            PrintStr("_formal");
            IncreaseOffset();
            PrintStr(formal.id.id);    // must be `PrintStr` to match original parser
            PrintSymbol(formal.type);
            DecreaseOffset();
        }

        void VisitAssignExpression(AssignExpression &expr) override {
            PrintLine(expr);
            PrintStr("_assign");
            IncreaseOffset();
            PrintStr((*expr.id).id);    // must be `PrintStr` to match original parser
            VisitExpression(*expr.expr);
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitStaticDispatchExpression(StaticDispatchExpression &expr) override {
            PrintLine(expr);
            PrintStr("_static_dispatch");
            IncreaseOffset();
            VisitExpression(*expr.expr);
            PrintSymbol(expr.type);
            PrintStr((*expr.id).id);    // must be `PrintStr` to match original parser
            PrintStr("(");
            for (const auto& arg: expr.list) {
                VisitExpression(*arg);
            }
            PrintStr(")");
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitDispatchExpression(DispatchExpression &expr) override {
            PrintLine(expr);
            PrintStr("_dispatch");
            IncreaseOffset();
            VisitExpression(*expr.expr);
            PrintStr((*expr.id).id);    // must be `PrintStr` to match original parser
            PrintStr("(");
            for (const auto& arg: expr.list) {
                VisitExpression(*arg);
            }
            PrintStr(")");
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitIfExpression(IfExpression &expr) override {
            PrintLine(expr);
            PrintStr("_cond");
            IncreaseOffset();
            VisitExpression(*expr.condition);
            VisitExpression(*expr.trueBranch);
            VisitExpression(*expr.falseBranch);
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitWhileExpression(WhileExpression &expr) override {
            PrintLine(expr);
            PrintStr("_loop");
            IncreaseOffset();
            VisitExpression(*expr.condition);
            VisitExpression(*expr.body);
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitBlockExpression(BlockExpression &expr) override {
            PrintLine(expr);
            PrintStr("_block");
            IncreaseOffset();
            for (const auto& exprInBlock: expr.list) {
                VisitExpression(*exprInBlock);
            }
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitLetExpression(LetExpression &expr) override {
            PrintLine(expr);
            PrintStr("_let");
            IncreaseOffset();
            PrintStr((*expr.id).id);    // must be `PrintStr` to match original parser
            PrintSymbol(expr.type);
            VisitExpression(*expr.expr);
            VisitExpression(*expr.body);
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitCaseExpression(CaseExpression &expr) override {
            PrintLine(expr);
            PrintStr("_typcase");
            IncreaseOffset();
            VisitExpression(*expr.expr);
            for (auto branch: expr.branches) {
                VisitCaseBranchExpression(branch);
            }
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitNewExpression(NewExpression &expr) override {
            PrintLine(expr);
            PrintStr("_new");
            IncreaseOffset();
            PrintSymbol(expr.type);
            DecreaseOffset();
            PrintType(expr);
        }

        template<class T>
        void VisitUnaryExpression(T &expr, const std::string& op) {
            PrintLine(expr);
            PrintStr(op);
            IncreaseOffset();
            VisitExpression(*expr.expr);
            DecreaseOffset();
            PrintType(expr);
        }

        template<class T>
        void VisitBinaryExpression(T &expr, const std::string& op) {
            PrintLine(expr);
            PrintStr(op);
            IncreaseOffset();
            VisitExpression(*expr.lhs);
            VisitExpression(*expr.rhs);
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitIsVoidExpression(IsVoidExpression &expr) override {
            VisitUnaryExpression(expr, "_isvoid");
        }

        void VisitPlusExpression(PlusExpression &expr) override {
            VisitBinaryExpression(expr, "_plus");
        }

        void VisitMinusExpression(MinusExpression &expr) override {
            VisitBinaryExpression(expr, "_sub");
        }

        void VisitMulExpression(MulExpression &expr) override {
            VisitBinaryExpression(expr, "_mul");
        }

        void VisitDivExpression(DivExpression &expr) override {
            VisitBinaryExpression(expr, "_divide");
        }

        void VisitInverseExpression(InverseExpression &expr) override {
            VisitUnaryExpression(expr, "_neg");
        }

        void VisitLessExpression(LessExpression &expr) override {
            VisitBinaryExpression(expr, "_lt");
        }

        void VisitLessOrEqualExpression(LessOrEqualExpression &expr) override {
            VisitBinaryExpression(expr, "_leq");
        }

        void VisitEqualExpression(EqualExpression &expr) override {
            VisitBinaryExpression(expr, "_eq");
        }

        void VisitNotExpression(NotExpression &expr) override {
            VisitUnaryExpression(expr, "_comp");
        }

        void VisitInBracketsExpression(InBracketsExpression &expr) override {
            VisitExpression(*expr.expr);
        }

        void VisitIntExpression(IntExpression &expr) override {
            PrintLine(expr);
            PrintStr("_int");
            IncreaseOffset();
            PrintStr(std::to_string(expr.value));
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitStringExpression(StringExpression &expr) override {
            PrintLine(expr);
            PrintStr("_string");
            IncreaseOffset();
            PrintStr(expr.value);
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitBoolExpression(BoolExpression &expr) override {
            PrintLine(expr);
            PrintStr("_bool");
            IncreaseOffset();
            PrintStr(std::to_string(expr.value));
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitIdExpression(IdExpression &expr) override {
            PrintLine(expr);
            PrintStr("_object");
            IncreaseOffset();
            stream << std::setw(static_cast<int>(offset + expr.id.length())) << expr.id << std::endl;
            DecreaseOffset();
            PrintType(expr);
        }

        void VisitNoExprExpression(NoExprExpression &expr) override {
            PrintLine(expr);
            stream << std::setw(static_cast<int>(offset + expr.value.length())) << expr.value << std::endl;
            PrintType(expr);
        }

    private:

        void VisitCaseBranchExpression(CaseBranchExpression &expr) override {
            PrintLine(expr);
            PrintStr("_branch");
            IncreaseOffset();
            PrintStr((*expr.id).id);    // must be `PrintStr` to match original parser
            PrintSymbol(expr.type);
            VisitExpression(*expr.expr);
            DecreaseOffset();
        }

    private:
        std::size_t offset = 0;
        std::stringstream stream;
    };
}