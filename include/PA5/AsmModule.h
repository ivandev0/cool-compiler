#pragma once

#include <utility>

#include "TypeEnvironment.h"
#include "ASTVisitor.h"
#include "Const.h"
#include "MIPS.h"
#include "Tables.h"
#include "Context.h"

namespace backend {
    class AsmModule: private parser::ASTVisitor<void> {
    public:
        explicit AsmModule(const semant::TypeEnvironment &type_env) : type_env_(type_env), context(Context(type_env)) {
            mips->SetDataMode();
            mips->data()->align(2);

            mips->SetHeapMode();
            mips->global(Names::heap_start)->label(Names::heap_start)->word(0)->text()
                ->global(Names::FormInit(Names::int_name))
                ->global(Names::FormInit(Names::str_name))
                ->global(Names::FormInit(Names::bool_name))
                ->global(Names::FormInit(Names::main))->global(Names::main_call);
        }

        void VisitProgram(parser::Program *program) override;

        std::string End() {
            mips->SetDataMode();
            SetUpBasicTags();
            ConfigureGC();
            for (const auto &item : prototypes_) {
                item.Serialize(mips);
            }
            SetUpConsts();
            SetUpClassNameTable();
            SetUpClassObjectTable();
            for (const auto &item : dispatch_tables_) {
                item.Serialize(mips);
            }

            mips->SetHeapMode();
            return mips->End();
        }

    private:
        std::string GetOrCreateConstFor(const std::string& str) {
            for (std::size_t i = 0; i < str_constants_.size(); ++i) {
                if (str_constants_[i].Match(str)) {
                    return "str_const" + std::to_string(i);
                }
            }
            str_constants_.emplace_back(str, GetTagFor(Names::str_name), str_constants_.size(), GetOrCreateConstFor(str.size()));
            return "str_const" + std::to_string(str_constants_.size() - 1);
        }

        std::string GetOrCreateConstFor(std::size_t value) {
            for (auto & int_constant : int_constants_) {
                if (int_constant.Match(value)) {
                    return "int_const" + std::to_string(value);
                }
            }
            int_constants_.emplace_back(value, GetTagFor(Names::int_name));
            return "int_const" + std::to_string(value);
        }

        std::string GetOrCreateConstFor(bool value) {
            for (auto & bool_constant : bool_constants_) {
                if (bool_constant.Match(value)) {
                    return "bool_const" + std::to_string((int) value);
                }
            }
            bool_constants_.emplace_back(value, GetTagFor(Names::bool_name));
            return "bool_const" + std::to_string((int) value);
        }

        std::size_t GetNextTag() { return tag++; }

    private:
        void BuildPrototype(const std::string& name) {
            std::vector<parser::AttrFeature> attrs = type_env_.class_table_.GetAllAttributesOf(name);
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

        void BuildInit(const parser::Class& klass) {
            mips->label(Names::FormInit(klass.type))->prolog(0);
            if (klass.type != Names::obj_name) {
                mips->jal(Names::FormInit(klass.parent));
                if (!type_env_.class_table_.IsBasicClass(klass.type)) {
                    std::vector<parser::AttrFeature> attrs = type_env_.class_table_.GetAttributesOf(klass.type);
                    for (std::size_t i = 0; i < attrs.size(); ++i) {
                        InitVariable(&*attrs[i].expr, attrs[i].type);
                        mips->sw(R::acc, R::s0.Shift(12 + 4 * i));
                    }
                }
            }
            mips->move(R::acc, R::s0)->epilog(0);
        }

        std::size_t GetTagFor(const std::string& type) {
            for (const auto &item : prototypes_) {
                if (item.GetName() == type) {
                    return item.GetTag();
                }
            }
            return 0; // TODO: maybe throw exception
        }

        void SetUpBasicTags() {
            mips->global("_int_tag")->global("_bool_tag")->global("_string_tag")
                ->label("_int_tag")->word(GetTagFor(Names::int_name))
                ->label("_bool_tag")->word(GetTagFor(Names::bool_name))
                ->label("_string_tag")->word(GetTagFor(Names::str_name));
        }

        void ConfigureGC() {
            mips->global("_MemMgr_INITIALIZER")->label("_MemMgr_INITIALIZER")->word(gc_enabled_ ? "_GenGC_Init" : "_NoGC_Init")
                ->global("_MemMgr_COLLECTOR")->label("_MemMgr_COLLECTOR")->word(gc_enabled_ ? "_GenGC_Collect" : "_NoGC_Collect")
                ->global("_MemMgr_TEST")->label("_MemMgr_TEST")->word(gc_test_enabled_ ? "1" : "0");
        }

        void SetUpConsts() {
            for (const auto &item : int_constants_) {
                item.Serialize(mips);
            }
            for (const auto &item : bool_constants_) {
                item.Serialize(mips);
            }
            for (const auto &item : str_constants_) {
                item.Serialize(mips);
            }
        }

        void SetUpClassNameTable() {
            mips->global(Names::name_tab)->label(Names::name_tab);
            for (const auto &item : prototypes_) {
                mips->word(GetOrCreateConstFor(item.GetName()));
            }
        }

        void SetUpClassObjectTable() {
            mips->label(Names::obj_tab);
            for (const auto &item : prototypes_) {
                mips->word(Names::FormProtObjName(item.GetName()))->word(Names::FormInit(item.GetName()));
            }
        }

        std::string NextLabel(const std::string& prefix = "") {
            return prefix + "_label_" + std::to_string(label_index_++);
        }

        std::size_t GetMethodOffset(const std::string& klass, const std::string& method) {
            for (const auto &table : dispatch_tables_) {
                if (table.IsFor(klass)) {
                    return table.GetMethodOffset(method);
                }
            }
            return -1;
        }

    private:
        void VisitClass(parser::Class *klass) override;
        void VisitAttrFeature(parser::AttrFeature *attrFeature) override { /*nothing*/ };
        void VisitMethodFeature(parser::MethodFeature *methodFeature) override;
        void VisitFormal(parser::Formal *formal) override { /*nothing*/ };
        void VisitAssignExpression(parser::AssignExpression *expr) override;
        void VisitStaticDispatchExpression(parser::StaticDispatchExpression *expr) override;
        void VisitDispatchExpression(parser::DispatchExpression *expr) override;
        void VisitIfExpression(parser::IfExpression *expr) override;
        void VisitWhileExpression(parser::WhileExpression *expr) override;
        void VisitBlockExpression(parser::BlockExpression *expr) override;
        void VisitLetExpression(parser::LetExpression *expr) override;
        void VisitCaseExpression(parser::CaseExpression *expr) override;
        void VisitNewExpression(parser::NewExpression *expr) override;
        void VisitIsVoidExpression(parser::IsVoidExpression *expr) override;
        void VisitPlusExpression(parser::PlusExpression *expr) override;
        void VisitMinusExpression(parser::MinusExpression *expr) override;
        void VisitMulExpression(parser::MulExpression *expr) override;
        void VisitDivExpression(parser::DivExpression *expr) override;
        void VisitInverseExpression(parser::InverseExpression *expr) override;
        void VisitLessExpression(parser::LessExpression *expr) override;
        void VisitLessOrEqualExpression(parser::LessOrEqualExpression *expr) override;
        void VisitEqualExpression(parser::EqualExpression *expr) override;
        void VisitNotExpression(parser::NotExpression *expr) override;
        void VisitInBracketsExpression(parser::InBracketsExpression *expr) override;
        void VisitIntExpression(parser::IntExpression *expr) override;
        void VisitStringExpression(parser::StringExpression *expr) override;
        void VisitBoolExpression(parser::BoolExpression *expr) override;
        void VisitIdExpression(parser::IdExpression *expr) override;
        void VisitNoExprExpression(parser::NoExprExpression *expr) override;
        void VisitCaseBranchExpression(parser::CaseBranchExpression *expr) override;

        void InitVariable(parser::Expression *expr, const std::string& type);
        void VisitCommonDispatchExpression(
            parser::Expression* expr, const std::string& name, const std::string& type, const std::vector<std::shared_ptr<parser::Expression>>& args
        );

    public:
        virtual ~AsmModule() {
            delete mips;
        }

    private:
        MIPS* mips = new MIPS();
        const semant::TypeEnvironment& type_env_;
        Context context;

        bool gc_enabled_ = true;
        bool gc_test_enabled_ = true;

        std::vector<IntConst> int_constants_;
        std::vector<BoolConst> bool_constants_;
        std::vector<StrConst> str_constants_;

        std::size_t tag = 0;
        std::size_t label_index_ = 0;
        std::vector<Prototype> prototypes_;
        std::vector<DispatchTable> dispatch_tables_;
    };
}
