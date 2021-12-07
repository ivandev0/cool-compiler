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
        static std::size_t CountUnescapedSymbols(const std::string& str) {
            std::size_t size = 0;
            if (!str.empty()) {
                size = 1;
                for (std::size_t i = 0; i < str.size() - 1; ++i, ++size) {
                    if (i != str.size() - 1 && str[i] == '\\') {
                        auto next = str[i + 1];
                        if (next == 'b' || next == 't' || next == 'n' || next == 'f' || next == '\\' || next == '"' || next == '\n') {
                            ++i;
                            --size;
                        }
                    }
                }
            }
            return size;
        }

        std::string GetOrCreateConstFor(const std::string& str) {
            for (std::size_t i = 0; i < str_constants_.size(); ++i) {
                if (str_constants_[i].Match(str)) {
                    return "str_const" + std::to_string(i);
                }
            }

            std::size_t size = CountUnescapedSymbols(str);
            str_constants_.emplace_back(str, GetTagFor(Names::str_name), str_constants_.size(), GetOrCreateConstFor(size));
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

    private:
        void SetUpTags(const std::vector<parser::Class>& classes) {
            auto tag = 0;
            for (const auto &item : classes) {
                tags_[item.type] = tag++;
            }
        }

        std::size_t GetTagFor(const std::string& type) const {
            auto type_tag = tags_.find(type);
            if (type_tag != tags_.end()) {
                return type_tag->second;
            }
            return 0; // TODO: maybe throw exception
        }


        std::size_t GetTagForLastChildOf(const std::string& type) const {
            auto begin = GetTagFor(type);
            if (begin == tags_.size() - 1) return begin;

            for (std::size_t i = begin + 1; i < tags_.size(); i++) {
                auto new_type = std::find_if(std::begin(tags_), std::end(tags_), [&](const auto& pair) {
                    return pair.second == i;
                });
                if (!type_env_.class_table_.CheckAIsSubtypeOfB(new_type->first, type)) {
                    return  i - 1;
                }
            }
            return tags_.size() - 1;
        }

        void BuildPrototype(const std::string& name) {
            std::vector<parser::AttrFeature> attrs = type_env_.class_table_.GetAllAttributesOf(name);
            std::vector<std::string> types;
            transform(attrs.begin(), attrs.end(), back_inserter(types), [](const auto& attr) { return attr.type; });
            prototypes_.emplace_back(name, GetTagFor(name), types);
        }

        void GetMethodsFor(const std::string& type, std::vector<std::pair<std::string, std::string>>& methods) {
            if (type != Names::obj_name) {
                GetMethodsFor(type_env_.class_table_.GetParentOf(type), methods);
            }

            auto klass = type_env_.class_table_.GetClassByName(type);
            for (auto feature : klass.features) {
                if (auto data = std::get_if<parser::MethodFeature>(&feature.feature)) {
                    auto overridden = false;
                    for (std::size_t i = 0; i < methods.size(); ++i) {
                        if (methods[i].second == data->id.id) {
                            overridden = true;
                            methods[i] = {klass.type, data->id.id};
                            break;
                        }
                    }
                    if (!overridden) {
                        methods.emplace_back(klass.type, data->id.id);
                    }
                }
            }
        }

        void BuildDispatchTable(const std::string& type) {
            std::vector<std::pair<std::string, std::string>> methods;
            GetMethodsFor(type, methods);
            dispatch_tables_.emplace_back(type, methods);
        }

        void BuildInit(const parser::Class& klass) {
            mips->label(Names::FormInit(klass.type))->prolog(0);
            if (klass.type != Names::obj_name) {
                mips->jal(Names::FormInit(klass.parent));
                if (!type_env_.class_table_.IsBasicClass(klass.type)) {
                    std::vector<parser::AttrFeature> attrs = type_env_.class_table_.GetAttributesOf(klass.type);
                    for (std::size_t i = 0; i < attrs.size(); ++i) {
                        InitVariable(&*attrs[i].expr, attrs[i].type);
                        auto offset = context.GetOffsetForAttr(attrs[i].id.id);
                        mips->sw(R::acc, R::s0.Shift(offset))->genGc(offset);
                    }
                }
            }
            mips->move(R::acc, R::s0)->epilog(0);
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
            parser::Expression* expr, const std::string& name, const std::string& type,
            const std::vector<std::shared_ptr<parser::Expression>>& args, bool is_static = false
        );
        void VisitBinaryArith(parser::Expression *left, parser::Expression *right, const std::string& op);
        void VisitCompExpression(parser::Expression *left, parser::Expression *right, const std::string& op);

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

        std::size_t label_index_ = 0;
        std::vector<Prototype> prototypes_;
        std::map<std::string, std::size_t> tags_;
        std::vector<DispatchTable> dispatch_tables_;
    };
}
