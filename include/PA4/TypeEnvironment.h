#pragma once
#include "MethodEnvironment.h"
#include "ObjectEnvironment.h"
#include "ClassTable.h"

namespace semant {
    class TypeEnvironment {
    public:
        explicit TypeEnvironment(const std::vector<parser::Class>& classes):
                class_table_(ClassTable(classes)), method_env_(MethodEnvironment(classes, class_table_)) {}

        void EnterClass(const parser::Class& klass) {
            self_type_ = klass.type;
            object_env_.EnterScope();
        }

        void ExitClass() {
            object_env_.ExitScope();
            self_type_ = "";
        }

        bool CheckAIsSubtypeOfB(const std::string& a, const std::string& b) const {
            if (a == "_no_type" || (a == ClassTable::self_type && b == ClassTable::self_type)) return true;

            return class_table_.CheckAIsSubtypeOfB(GetActualType(a), GetActualType(b));
        }

        bool IsDefined(const std::string& type) const {
            if (type == ClassTable::self_type) return true;
            return class_table_.IsDefined(type);
        }

        bool IsNotDefined(const std::string& type) const {
            return !IsDefined(type);
        }

        std::string Combine(std::vector<std::string>& types) const {
            if (types.empty()) return class_table_.object_class_.type;

            for (auto& type : types) {
                if (type == ClassTable::self_type) type = self_type_;
            }

            auto common = types[0];
            for (std::size_t i = 1; i < types.size(); ++i) {
                common = class_table_.CommonSuperType(common, types[i]);
            }
            return common;
        }

    private:
        std::string GetActualType(const std::string& type) const {
            if (type == ClassTable::self_type) return self_type_;
            return type;
        }

    public:
        ClassTable class_table_;
        MethodEnvironment method_env_;
        ObjectEnvironment object_env_;
        std::string self_type_;
    };
}