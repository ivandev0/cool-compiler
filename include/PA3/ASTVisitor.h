#pragma once
#include <stdexcept>
#include "Nodes.h"

namespace parser {
    template<typename R>
    class ASTVisitor {
    public:
        virtual R visitProgram(Program &program) = 0;

    protected:
        R visitFeature(Feature &feature) {
            if (auto data = std::get_if<AttrFeature>(&(feature.feature))) return visitAttrFeature(*data);
            if (auto data = std::get_if<MethodFeature>(&(feature.feature))) return visitMethodFeature(*data);

            throw std::runtime_error("Unsupported feature type");

        }
        R visitExpression(Expression &expr) {
            if (auto data = std::get_if<AssignExpression>(&(expr.data))) return visitAssignExpression(*data);
            if (auto data = std::get_if<StaticDispatchExpression>(&(expr.data))) return visitStaticDispatchExpression(*data);
            if (auto data = std::get_if<DispatchExpression>(&(expr.data))) return visitDispatchExpression(*data);
            if (auto data = std::get_if<IfExpression>(&(expr.data))) return visitIfExpression(*data);
            if (auto data = std::get_if<WhileExpression>(&(expr.data))) return visitWhileExpression(*data);
            if (auto data = std::get_if<BlockExpression>(&(expr.data))) return visitBlockExpression(*data);
            if (auto data = std::get_if<LetExpression>(&(expr.data))) return visitLetExpression(*data);
            if (auto data = std::get_if<CaseExpression>(&(expr.data))) return visitCaseExpression(*data);
            if (auto data = std::get_if<NewExpression>(&(expr.data))) return visitNewExpression(*data);
            if (auto data = std::get_if<IsVoidExpression>(&(expr.data))) return visitIsVoidExpression(*data);
            if (auto data = std::get_if<PlusExpression>(&(expr.data))) return visitPlusExpression(*data);
            if (auto data = std::get_if<MinusExpression>(&(expr.data))) return visitMinusExpression(*data);
            if (auto data = std::get_if<MulExpression>(&(expr.data))) return visitMulExpression(*data);
            if (auto data = std::get_if<DivExpression>(&(expr.data))) return visitDivExpression(*data);
            if (auto data = std::get_if<InverseExpression>(&(expr.data))) return visitInverseExpression(*data);
            if (auto data = std::get_if<LessExpression>(&(expr.data))) return visitLessExpression(*data);
            if (auto data = std::get_if<LessOrEqualExpression>(&(expr.data))) return visitLessOrEqualExpression(*data);
            if (auto data = std::get_if<EqualExpression>(&(expr.data))) return visitEqualExpression(*data);
            if (auto data = std::get_if<NotExpression>(&(expr.data))) return visitNotExpression(*data);
            if (auto data = std::get_if<InBracketsExpression>(&(expr.data))) return visitInBracketsExpression(*data);
            if (auto data = std::get_if<IntExpression>(&(expr.data))) return visitIntExpression(*data);
            if (auto data = std::get_if<StringExpression>(&(expr.data))) return visitStringExpression(*data);
            if (auto data = std::get_if<BoolExpression>(&(expr.data))) return visitBoolExpression(*data);
            if (auto data = std::get_if<IdExpression>(&(expr.data))) return visitIdExpression(*data);
            if (auto data = std::get_if<NoExprExpression>(&(expr.data))) return visitNoExprExpression(*data);

            throw std::runtime_error("Unsupported expression type");
        }
        virtual R visitClass(Class &klass) = 0;
        virtual R visitAttrFeature(AttrFeature &attrFeature) = 0;
        virtual R visitMethodFeature(MethodFeature &methodFeature) = 0;

        virtual R visitFormal(Formal &formal) = 0;
        virtual R visitAssignExpression(AssignExpression &expr) = 0;
        virtual R visitStaticDispatchExpression(StaticDispatchExpression &expr) = 0;
        virtual R visitDispatchExpression(DispatchExpression &expr) = 0;
        virtual R visitIfExpression(IfExpression &expr) = 0;
        virtual R visitWhileExpression(WhileExpression &expr) = 0;
        virtual R visitBlockExpression(BlockExpression &expr) = 0;
        virtual R visitLetExpression(LetExpression &expr) = 0;
        virtual R visitCaseExpression(CaseExpression &expr) = 0;
        virtual R visitNewExpression(NewExpression &expr) = 0;
        virtual R visitIsVoidExpression(IsVoidExpression &expr) = 0;
        virtual R visitPlusExpression(PlusExpression &expr) = 0;
        virtual R visitMinusExpression(MinusExpression &expr) = 0;
        virtual R visitMulExpression(MulExpression &expr) = 0;
        virtual R visitDivExpression(DivExpression &expr) = 0;
        virtual R visitInverseExpression(InverseExpression &expr) = 0;
        virtual R visitLessExpression(LessExpression &expr) = 0;
        virtual R visitLessOrEqualExpression(LessOrEqualExpression &expr) = 0;
        virtual R visitEqualExpression(EqualExpression &expr) = 0;
        virtual R visitNotExpression(NotExpression &expr) = 0;
        virtual R visitInBracketsExpression(InBracketsExpression &expr) = 0;
        virtual R visitIntExpression(IntExpression &expr) = 0;
        virtual R visitStringExpression(StringExpression &expr) = 0;
        virtual R visitBoolExpression(BoolExpression &expr) = 0;
        virtual R visitIdExpression(IdExpression &expr) = 0;
        virtual R visitNoExprExpression(NoExprExpression &expr) = 0;

        virtual R visitCaseBranchExpression(CaseBranchExpression &expr) = 0;
    };
}
