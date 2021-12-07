#pragma once
#include "ASTVisitor.h"
#include "Nodes.h"
#include "ClassTable.h"
#include "TypeEnvironment.h"

namespace semant {
    class SemanticAnalyzer : private parser::ASTVisitor<std::string> {
    public:
        void Analyze(parser::Program* program);
        TypeEnvironment& GetTypeEnvironment() { return *env_; }

        virtual ~SemanticAnalyzer() {
            delete env_;
        }

    private:
        std::string VisitProgram(parser::Program *program) override;
        std::string VisitClass(parser::Class *klass) override;
        std::string VisitAttrFeature(parser::AttrFeature *attrFeature) override;
        std::string VisitMethodFeature(parser::MethodFeature *methodFeature) override;
        std::string VisitFormal(parser::Formal *formal) override;
        std::string VisitAssignExpression(parser::AssignExpression *expr) override;
        std::string VisitStaticDispatchExpression(parser::StaticDispatchExpression *expr) override;
        std::string VisitDispatchExpression(parser::DispatchExpression *expr) override;
        std::string VisitIfExpression(parser::IfExpression *expr) override;
        std::string VisitWhileExpression(parser::WhileExpression *expr) override;
        std::string VisitBlockExpression(parser::BlockExpression *expr) override;
        std::string VisitLetExpression(parser::LetExpression *expr) override;
        std::string VisitCaseExpression(parser::CaseExpression *expr) override;
        std::string VisitNewExpression(parser::NewExpression *expr) override;
        std::string VisitIsVoidExpression(parser::IsVoidExpression *expr) override;
        std::string VisitPlusExpression(parser::PlusExpression *expr) override;
        std::string VisitMinusExpression(parser::MinusExpression *expr) override;
        std::string VisitMulExpression(parser::MulExpression *expr) override;
        std::string VisitDivExpression(parser::DivExpression *expr) override;
        std::string VisitInverseExpression(parser::InverseExpression *expr) override;
        std::string VisitLessExpression(parser::LessExpression *expr) override;
        std::string VisitLessOrEqualExpression(parser::LessOrEqualExpression *expr) override;
        std::string VisitEqualExpression(parser::EqualExpression *expr) override;
        std::string VisitNotExpression(parser::NotExpression *expr) override;
        std::string VisitInBracketsExpression(parser::InBracketsExpression *expr) override;
        std::string VisitIntExpression(parser::IntExpression *expr) override;
        std::string VisitStringExpression(parser::StringExpression *expr) override;
        std::string VisitBoolExpression(parser::BoolExpression *expr) override;
        std::string VisitIdExpression(parser::IdExpression *expr) override;
        std::string VisitNoExprExpression(parser::NoExprExpression *expr) override;
        std::string VisitCaseBranchExpression(parser::CaseBranchExpression *expr) override;

        std::string VisitExpression(parser::Expression *expr) override {
            expr->result_type = parser::ASTVisitor<std::string>::VisitExpression(expr);
            return expr->result_type;
        }

        template<class T>
        std::string VisitBinaryExpression(T *expr, const std::string& op);

        void VisitParentAttrFeature(const std::string &parent);
        void VisitAttrFeatures(const std::string& type);

    private:
        static bool ContainsNoExpr(const std::shared_ptr<parser::Expression>& expr) {
            return std::get_if<parser::NoExprExpression>(&expr->data);
        }

        void CheckMain() {
            if (env_->IsNotDefined("Main")) {
                throw std::runtime_error("Class Main is not defined.");
            }
            auto main_sig = env_->method_env_.GetMethodSignature("Main", "main");
            if (main_sig == nullptr) {
                throw std::runtime_error("No 'main' method in class Main.");
            }

            if (main_sig->types.size() != 1) {
                throw std::runtime_error("'main' method in class Main should have no arguments");
            }
        }

        void CheckAttributesAreUnique(const parser::Class& klass) const {
            std::vector<std::string> attrs;
            for (auto feature : klass.features) {
                if (auto data = std::get_if<parser::AttrFeature>(&feature.feature)) {
                    if (std::find(attrs.begin(), attrs.end(), data->id.id) != attrs.end()) {
                        throw std::runtime_error("Attribute " + data->id.id + " is multiply defined in class.");
                    }
                    attrs.push_back(data->id.id);
                }
            }
        }

        void CheckAttributesAreUniqueFromParent(const parser::Class& klass) const {
            std::vector<std::string> attrs;
            for (auto feature : klass.features) {
                if (auto data = std::get_if<parser::AttrFeature>(&feature.feature)) {
                    if (env_->object_env_.IsDefined(data->id.id)) {
                        throw std::runtime_error("Attribute " + data->id.id + " is an attribute of an inherited class.");
                    }
                    attrs.push_back(data->id.id);
                }
            }
        }

    private:
        TypeEnvironment* env_ = nullptr;
    };
}
