#pragma once
#include <stdexcept>
#include "Nodes.h"

namespace parser {
    template<typename R>
    class ASTVisitor {
    public:
        virtual R VisitProgram(Program *program) = 0;

    protected:
        virtual R VisitFeature(Feature *feature) {
            if (auto data = std::get_if<AttrFeature>(&feature->feature)) return VisitAttrFeature(data);
            if (auto data = std::get_if<MethodFeature>(&feature->feature)) return VisitMethodFeature(data);

            throw std::runtime_error("Unsupported feature type");

        }
        virtual R VisitExpression(Expression *expr) {
            if (auto data = std::get_if<AssignExpression>(&(expr->data))) return VisitAssignExpression(data);
            if (auto data = std::get_if<StaticDispatchExpression>(&(expr->data))) return VisitStaticDispatchExpression(data);
            if (auto data = std::get_if<DispatchExpression>(&(expr->data))) return VisitDispatchExpression(data);
            if (auto data = std::get_if<IfExpression>(&(expr->data))) return VisitIfExpression(data);
            if (auto data = std::get_if<WhileExpression>(&(expr->data))) return VisitWhileExpression(data);
            if (auto data = std::get_if<BlockExpression>(&(expr->data))) return VisitBlockExpression(data);
            if (auto data = std::get_if<LetExpression>(&(expr->data))) return VisitLetExpression(data);
            if (auto data = std::get_if<CaseExpression>(&(expr->data))) return VisitCaseExpression(data);
            if (auto data = std::get_if<NewExpression>(&(expr->data))) return VisitNewExpression(data);
            if (auto data = std::get_if<IsVoidExpression>(&(expr->data))) return VisitIsVoidExpression(data);
            if (auto data = std::get_if<PlusExpression>(&(expr->data))) return VisitPlusExpression(data);
            if (auto data = std::get_if<MinusExpression>(&(expr->data))) return VisitMinusExpression(data);
            if (auto data = std::get_if<MulExpression>(&(expr->data))) return VisitMulExpression(data);
            if (auto data = std::get_if<DivExpression>(&(expr->data))) return VisitDivExpression(data);
            if (auto data = std::get_if<InverseExpression>(&(expr->data))) return VisitInverseExpression(data);
            if (auto data = std::get_if<LessExpression>(&(expr->data))) return VisitLessExpression(data);
            if (auto data = std::get_if<LessOrEqualExpression>(&(expr->data))) return VisitLessOrEqualExpression(data);
            if (auto data = std::get_if<EqualExpression>(&(expr->data))) return VisitEqualExpression(data);
            if (auto data = std::get_if<NotExpression>(&(expr->data))) return VisitNotExpression(data);
            if (auto data = std::get_if<InBracketsExpression>(&(expr->data))) return VisitInBracketsExpression(data);
            if (auto data = std::get_if<IntExpression>(&(expr->data))) return VisitIntExpression(data);
            if (auto data = std::get_if<StringExpression>(&(expr->data))) return VisitStringExpression(data);
            if (auto data = std::get_if<BoolExpression>(&(expr->data))) return VisitBoolExpression(data);
            if (auto data = std::get_if<IdExpression>(&(expr->data))) return VisitIdExpression(data);
            if (auto data = std::get_if<NoExprExpression>(&(expr->data))) return VisitNoExprExpression(data);

            throw std::runtime_error("Unsupported expression type");
        }
        virtual R VisitClass(Class *klass) = 0;
        virtual R VisitAttrFeature(AttrFeature *attrFeature) = 0;
        virtual R VisitMethodFeature(MethodFeature *methodFeature) = 0;

        virtual R VisitFormal(Formal *formal) = 0;
        virtual R VisitAssignExpression(AssignExpression *expr) = 0;
        virtual R VisitStaticDispatchExpression(StaticDispatchExpression *expr) = 0;
        virtual R VisitDispatchExpression(DispatchExpression *expr) = 0;
        virtual R VisitIfExpression(IfExpression *expr) = 0;
        virtual R VisitWhileExpression(WhileExpression *expr) = 0;
        virtual R VisitBlockExpression(BlockExpression *expr) = 0;
        virtual R VisitLetExpression(LetExpression *expr) = 0;
        virtual R VisitCaseExpression(CaseExpression *expr) = 0;
        virtual R VisitNewExpression(NewExpression *expr) = 0;
        virtual R VisitIsVoidExpression(IsVoidExpression *expr) = 0;
        virtual R VisitPlusExpression(PlusExpression *expr) = 0;
        virtual R VisitMinusExpression(MinusExpression *expr) = 0;
        virtual R VisitMulExpression(MulExpression *expr) = 0;
        virtual R VisitDivExpression(DivExpression *expr) = 0;
        virtual R VisitInverseExpression(InverseExpression *expr) = 0;
        virtual R VisitLessExpression(LessExpression *expr) = 0;
        virtual R VisitLessOrEqualExpression(LessOrEqualExpression *expr) = 0;
        virtual R VisitEqualExpression(EqualExpression *expr) = 0;
        virtual R VisitNotExpression(NotExpression *expr) = 0;
        virtual R VisitInBracketsExpression(InBracketsExpression *expr) = 0;
        virtual R VisitIntExpression(IntExpression *expr) = 0;
        virtual R VisitStringExpression(StringExpression *expr) = 0;
        virtual R VisitBoolExpression(BoolExpression *expr) = 0;
        virtual R VisitIdExpression(IdExpression *expr) = 0;
        virtual R VisitNoExprExpression(NoExprExpression *expr) = 0;

        virtual R VisitCaseBranchExpression(CaseBranchExpression *expr) = 0;
    };
}
