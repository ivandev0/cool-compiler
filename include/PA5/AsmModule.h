#pragma once

#include <utility>

#include "TypeEnvironment.h"
#include "ASTVisitor.h"
#include "Const.h"

namespace backend {
    struct Prototype {
    public:
        Prototype(std::string name, std::size_t tag, std::vector<std::string> types) :
            name(std::move(name)), tag(tag), types(std::move(types)) {}
        std::string ToData() const;
        std::string GetName() const { return name; }
        std::size_t GetTag() const { return tag; }
    private:
        std::string name;
        std::size_t tag;
        std::vector<std::string> types;
    };

    struct DispatchTable {
        DispatchTable(std::string name, std::vector<std::pair<std::string, std::string>> methods) :
            name(std::move(name)), methods(std::move(methods)) {}

        std::string ToData() const;
    private:
        std::string name;
        std::vector<std::pair<std::string, std::string>> methods;
    };

    class AsmModule: private parser::ASTVisitor<void> {
    public:
        explicit AsmModule(const semant::TypeEnvironment &typeEnv) : type_env_(typeEnv) {}

        void VisitProgram(parser::Program *program) override;

        std::size_t GetNextTag() { return tag++; }

        std::string ToData() {
            std::stringstream result;
            result << "\t.data\n" << "\t.align\t2\n";
            result << SetUpBasicTags() << "\n";
            result << ConfigureGC() << "\n";
            for (const auto &item : prototypes_) {
                result << item.ToData() << "\n";
            }
            result << SetUpConsts() << "\n";
            result << SetUpClassNameTable() << "\n";
            result << SetUpClassObjectTable() << "\n";
            for (const auto &item : dispatch_tables_) {
                result << item.ToData() << "\n";
            }
            return result.str();
        }

    public:
        std::string GetOrCreateConstFor(const std::string& str) {
            for (std::size_t i = 0; i < str_constants_.size(); ++i) {
                if (str_constants_[i].Match(str)) {
                    return "str_const" + std::to_string(i);
                }
            }
            str_constants_.emplace_back(str, GetTagFor("String"), str_constants_.size(), GetOrCreateConstFor(str.size()));
            return "str_const" + std::to_string(str_constants_.size() - 1);
        }

        std::string GetOrCreateConstFor(std::size_t value) {
            for (auto & int_constant : int_constants_) {
                if (int_constant.Match(value)) {
                    return "int_const" + std::to_string(value);
                }
            }
            int_constants_.emplace_back(value, GetTagFor("Int"));
            return "int_const" + std::to_string(value);
        }

        std::string GetOrCreateConstFor(bool value) {
            for (auto & bool_constant : bool_constants_) {
                if (bool_constant.Match(value)) {
                    return "bool_const" + std::to_string((int) value);
                }
            }
            bool_constants_.emplace_back(value, GetTagFor("Bool"));
            return "bool_const" + std::to_string((int) value);
        }

    private:
        void BuildPrototype(const std::string& name) {
            std::vector<parser::AttrFeature> attrs = type_env_.class_table_.GetAttributesOf(name);
            std::vector<std::string> types;
            transform(attrs.begin(), attrs.end(), back_inserter(types), [](const auto& attr) { return attr.type; });
            prototypes_.emplace_back(name, GetNextTag(), types);
        }

        void BuildDispatchTable(const std::string& type) {
            auto methods = type_env_.method_env_.GetMethodsFor(type);
            std::vector<std::pair<std::string, std::string>> names;
            transform(methods.begin(), methods.end(), back_inserter(names), [](const auto& method) {
                return std::make_pair(method.original_type, method.method_name);
            });
            dispatch_tables_.emplace_back(type, names);
        }

        std::size_t GetTagFor(const std::string& type) {
            for (const auto &item : prototypes_) {
                if (item.GetName() == type) {
                    return item.GetTag();
                }
            }
            return 0; // TODO: maybe throw exception
        }

        std::string SetUpBasicTags() {
            std::stringstream result;
            result << "\t.globl\t_int_tag\n";
            result << "\t.globl\t_bool_tag\n";
            result << "\t.globl\t_string_tag\n";

            result << "_int_tag:\n";
            result << "\t.word\t" << GetTagFor("Int") << "\n";

            result << "_bool_tag:\n";
            result << "\t.word\t" << GetTagFor("Bool") << "\n";

            result << "_string_tag:\n";
            result << "\t.word\t" << GetTagFor("String") << "\n";
            return result.str();
        }

        std::string ConfigureGC() {
            std::stringstream result;
            result << "\t.globl\t_MemMgr_INITIALIZER\n";
            result << "_MemMgr_INITIALIZER:\n";
            result << "\t.word\t_NoGC_Init\n";

            result << "\t.globl\t_MemMgr_COLLECTOR\n";
            result << "_MemMgr_COLLECTOR:\n";
            result << "\t.word\t_NoGC_Collect\n";

            result << "\t.globl\t_MemMgr_TEST\n";
            result << "_MemMgr_TEST:\n";
            result << "\t.word\t0";
            return result.str();
        }

        std::string SetUpConsts() {
            std::stringstream result;
            for (const auto &item : int_constants_) {
                result << item.ToData() << "\n";
            }
            for (const auto &item : bool_constants_) {
                result << item.ToData() << "\n";
            }
            for (const auto &item : str_constants_) {
                result << item.ToData() << "\n";
            }
            return result.str();
        }

        std::string SetUpClassNameTable() {
            std::stringstream result;
            result << "\t.globl\tclass_nameTab\n";
            result << "class_nameTab:\n";
            for (const auto &item : prototypes_) {
                result << "\t.word\t" << GetOrCreateConstFor(item.GetName()) << "\n";
            }
            return result.str();
        }

        std::string SetUpClassObjectTable() {
            std::stringstream result;
            result << "class_objTab:\n";
            for (const auto &item : prototypes_) {
                result << "\t.word\t" << item.GetName() << "_protObj\n";
                result << "\t.word\t" << item.GetName() << "_init\n";
            }
            return result.str();
        }

    private:
        void VisitClass(parser::Class *klass) override;
        void VisitAttrFeature(parser::AttrFeature *attrFeature) override {}
        void VisitMethodFeature(parser::MethodFeature *methodFeature) override {};
        void VisitFormal(parser::Formal *formal) override {};
        void VisitAssignExpression(parser::AssignExpression *expr) override {};
        void VisitStaticDispatchExpression(parser::StaticDispatchExpression *expr) override {};
        void VisitDispatchExpression(parser::DispatchExpression *expr) override {};
        void VisitIfExpression(parser::IfExpression *expr) override {};
        void VisitWhileExpression(parser::WhileExpression *expr) override {};
        void VisitBlockExpression(parser::BlockExpression *expr) override {};
        void VisitLetExpression(parser::LetExpression *expr) override {};
        void VisitCaseExpression(parser::CaseExpression *expr) override {};
        void VisitNewExpression(parser::NewExpression *expr) override {};
        void VisitIsVoidExpression(parser::IsVoidExpression *expr) override {};
        void VisitPlusExpression(parser::PlusExpression *expr) override {};
        void VisitMinusExpression(parser::MinusExpression *expr) override {};
        void VisitMulExpression(parser::MulExpression *expr) override {};
        void VisitDivExpression(parser::DivExpression *expr) override {};
        void VisitInverseExpression(parser::InverseExpression *expr) override {};
        void VisitLessExpression(parser::LessExpression *expr) override {};
        void VisitLessOrEqualExpression(parser::LessOrEqualExpression *expr) override {};
        void VisitEqualExpression(parser::EqualExpression *expr) override {};
        void VisitNotExpression(parser::NotExpression *expr) override {};
        void VisitInBracketsExpression(parser::InBracketsExpression *expr) override {};
        void VisitIntExpression(parser::IntExpression *expr) override {};
        void VisitStringExpression(parser::StringExpression *expr) override {};
        void VisitBoolExpression(parser::BoolExpression *expr) override {};
        void VisitIdExpression(parser::IdExpression *expr) override {};
        void VisitNoExprExpression(parser::NoExprExpression *expr) override {};
        void VisitCaseBranchExpression(parser::CaseBranchExpression *expr) override {};

    private:
        const semant::TypeEnvironment& type_env_;
        std::vector<IntConst> int_constants_;
        std::vector<BoolConst> bool_constants_;
        std::vector<StrConst> str_constants_;

        std::size_t tag = 0;
        std::vector<Prototype> prototypes_;
        std::vector<DispatchTable> dispatch_tables_;
    };
}
