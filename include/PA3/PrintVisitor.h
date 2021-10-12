#pragma once
#include <string>
#include "ASTVisitor.h"

namespace parser {
    class PrintVisitor: public ASTVisitor<void> {
    public:
        void visitProgram(parser::Program &program) override {
            printLine(program);
            printStr("_program");
            increaseOffset();
            for (auto klass: program.classes) {
                visitClass(klass);
            }
            decreaseOffset();
        }

        std::string getResult() {
            return stream.str();
        }

    private:
        void increaseOffset() { offset += 2; }
        void decreaseOffset() { offset -= 2; }

        void printLine(const TreeNode &node) {
            auto lineNumber = std::to_string(node.lineNumber);
            stream << std::setw(static_cast<int>(offset + lineNumber.length())) << "#" << lineNumber << std::endl;
        }

        void printStr(const std::string &name) {
            stream << std::setw(static_cast<int>(offset + name.length())) << name << std::endl;
        }

        void printType(const TreeNode &node) {
            stream << ":" << std::setw(static_cast<int>(offset + node.resultType.length())) << node.resultType << std::endl;
        }

        // TODO these methods can be in handy if we introduce new type - Symbol
        void printSymbol(const std::string &symbol) {
            if (symbol.empty()) return;
            stream << std::setw(static_cast<int>(offset + symbol.length())) << symbol << std::endl;
        }

        void visitClass(Class &klass) override {
            printLine(klass);
            printStr("_class");
            printSymbol(klass.type);
            printSymbol(klass.parent);
            printStr("\"" + klass.filename + "\"");
            printStr("(");
            increaseOffset();
            for (auto feature: klass.features) {
                visitFeature(feature);
            }
            decreaseOffset();
            printStr(")");
        }

        void visitAttrFeature(AttrFeature &attrFeature) override {
            printLine(attrFeature);
            printStr("_attr");
            increaseOffset();
            visitIdExpression(attrFeature.id);
            printSymbol(attrFeature.type);
            visitExpression(*attrFeature.expr);
            decreaseOffset();
        }

        void visitMethodFeature(MethodFeature &methodFeature) override {
            printLine(methodFeature);
            printStr("_method");
            increaseOffset();
            visitIdExpression(methodFeature.id);
            for (auto param: methodFeature.params) {
                visitFormal(param);
            }
            printSymbol(methodFeature.returnType);
            visitExpression(*methodFeature.expr);
            decreaseOffset();
        }

        void visitFormal(Formal &formal) override {
            printLine(formal);
            printStr("_formal");
            increaseOffset();
            visitIdExpression(formal.id);
            printSymbol(formal.type);
            decreaseOffset();
        }

        void visitAssignExpression(AssignExpression &expr) override {
            printLine(expr);
            printStr("_assign");
            increaseOffset();
            visitIdExpression(*expr.id);
            visitExpression(*expr.expr);
            decreaseOffset();
            printType(expr);
        }

        void visitStaticDispatchExpression(StaticDispatchExpression &expr) override {
            printLine(expr);
            printStr("_static_dispatch");
            increaseOffset();
            visitExpression(*expr.expr);
            printSymbol(expr.type);
            visitIdExpression(*expr.id);
            printStr("(");
            for (const auto& arg: expr.list) {
                visitExpression(*arg);
            }
            printStr(")");
            decreaseOffset();
            printType(expr);
        }

        void visitDispatchExpression(DispatchExpression &expr) override {
            printLine(expr);
            printStr("_dispatch");
            increaseOffset();
            visitExpression(*expr.expr);
            visitIdExpression(*expr.id);
            printStr("(");
            for (const auto& arg: expr.list) {
                visitExpression(*arg);
            }
            printStr(")");
            decreaseOffset();
            printType(expr);
        }

        void visitIfExpression(IfExpression &expr) override {
            printLine(expr);
            printStr("_cond");
            increaseOffset();
            visitExpression(*expr.condition);
            visitExpression(*expr.trueBranch);
            visitExpression(*expr.falseBranch);
            decreaseOffset();
            printType(expr);
        }

        void visitWhileExpression(WhileExpression &expr) override {
            printLine(expr);
            printStr("_loop");
            increaseOffset();
            visitExpression(*expr.condition);
            visitExpression(*expr.body);
            decreaseOffset();
            printType(expr);
        }

        void visitBlockExpression(BlockExpression &expr) override {
            printLine(expr);
            printStr("_block");
            for (const auto& exprInBlock: expr.list) {
                visitExpression(*exprInBlock);
            }
            printType(expr);
        }

        void visitLetExpression(LetExpression &expr) override {
            for (auto letStmt: expr.statements) {
                visitLetStatementExpression(letStmt);
            }
            visitExpression(*expr.body);
            for (auto letStmt: expr.statements) {
                decreaseOffset();
            }

            printType(expr);
        }

        void visitCaseExpression(CaseExpression &expr) override {
            printLine(expr);
            printStr("_typecase");
            increaseOffset();
            visitExpression(*expr.expr);
            for (auto branch: expr.branches) {
                visitCaseBranchExpression(branch);
            }
            decreaseOffset();
            printType(expr);
        }

        void visitNewExpression(NewExpression &expr) override {
            printLine(expr);
            printStr("_new");
            increaseOffset();
            printSymbol(expr.type);
            decreaseOffset();
            printType(expr);
        }

        template<class T>
        void visitUnaryExpression(T &expr, const std::string& op) {
            printLine(expr);
            printStr(op);
            increaseOffset();
            visitExpression(*expr.expr);
            decreaseOffset();
            printType(expr);
        }

        template<class T>
        void visitBinaryExpression(T &expr, const std::string& op) {
            printLine(expr);
            printStr(op);
            increaseOffset();
            visitExpression(*expr.lhs);
            visitExpression(*expr.rhs);
            decreaseOffset();
            printType(expr);
        }

        void visitIsVoidExpression(IsVoidExpression &expr) override {
            visitUnaryExpression(expr, "_isvoid");
        }

        void visitPlusExpression(PlusExpression &expr) override {
            visitBinaryExpression(expr, "_plus");
        }

        void visitMinusExpression(MinusExpression &expr) override {
            visitBinaryExpression(expr, "_sub");
        }

        void visitMulExpression(MulExpression &expr) override {
            visitBinaryExpression(expr, "_mul");
        }

        void visitDivExpression(DivExpression &expr) override {
            visitBinaryExpression(expr, "_divide");
        }

        void visitInverseExpression(InverseExpression &expr) override {
            visitUnaryExpression(expr, "_neg");
        }

        void visitLessExpression(LessExpression &expr) override {
            visitBinaryExpression(expr, "_lt");
        }

        void visitLessOrEqualExpression(LessOrEqualExpression &expr) override {
            visitBinaryExpression(expr, "_leq");
        }

        void visitEqualExpression(EqualExpression &expr) override {
            visitBinaryExpression(expr, "_eq");
        }

        void visitNotExpression(NotExpression &expr) override {
            visitUnaryExpression(expr, "_comp");
        }

        void visitInBracketsExpression(InBracketsExpression &expr) override {
            visitExpression(*expr.expr);
        }

        void visitIntExpression(IntExpression &expr) override {
            printLine(expr);
            printStr("_int");
            increaseOffset();
            printStr(std::to_string(expr.value));
            decreaseOffset();
            printType(expr);
        }

        void visitStringExpression(StringExpression &expr) override {
            printLine(expr);
            printStr("_string");
            increaseOffset();
            printStr("\"" + expr.value + "\"");
            decreaseOffset();
            printType(expr);
        }

        void visitBoolExpression(BoolExpression &expr) override {
            printLine(expr);
            printStr("_bool");
            increaseOffset();
            printStr(std::to_string(expr.value));
            decreaseOffset();
            printType(expr);
        }

        void visitIdExpression(IdExpression &expr) override {
            stream << std::setw(static_cast<int>(offset + expr.id.length())) << expr.id << std::endl;
        }

        void visitLetStatementExpression(LetStatementExpression &expr) override {
            printLine(expr);
            printStr("_let");
            increaseOffset();
            visitIdExpression(*expr.id);
            printSymbol(expr.type);
            visitExpression(*expr.expr);
            printType(expr);
        }

        void visitCaseBranchExpression(CaseBranchExpression &expr) override {
            printLine(expr);
            printStr("_branch");
            visitIdExpression(*expr.id);
            printSymbol(expr.type);
            visitExpression(*expr.expr);
            printType(expr);
        }

    private:
        std::size_t offset = 0;
        std::stringstream stream;
    };
}