#pragma once
#include "MIPS.h"

namespace backend {
    struct Prototype: public Serializable {
    public:
        Prototype(std::string name, std::size_t tag, std::vector<std::string> types) :
                name(std::move(name)), tag(tag), types(std::move(types)) {}

        std::string GetName() const { return name; }
        std::size_t GetTag() const { return tag; }

        void Serialize(MIPS* mips) const override {
            mips->global(Names::FormProtObjName(name))
                ->word("-1")
                ->label(Names::FormProtObjName(name))
                ->word(tag)
                ->word(types.size() + 3)
                ->word(Names::FormDispTableName(name));

            for (const auto &type : types) {
                if (type == Names::str_name && name != Names::str_name) {
                    mips->word("str_const0");
                } else if (type == Names::int_name && name != Names::int_name) {
                    mips->word("int_const0");
                } else if (type == Names::bool_name && name != Names::bool_name) {
                    mips->word("bool_const0");
                } else {
                    mips->word("0");
                }
            }
        }

    private:
        std::string name;
        std::size_t tag;
        std::vector<std::string> types;
    };

    struct DispatchTable: public Serializable {
        DispatchTable(std::string name, std::vector<std::pair<std::string, std::string>> methods) :
                name(std::move(name)), methods(std::move(methods)) {}

        bool IsFor(const std::string& class_name) const {
            return class_name == this->name;
        }

        std::size_t GetMethodOffset(const std::string& method_name) const {
            for (std::size_t i = 0; i < methods.size(); ++i) {
                if (methods[i].second == method_name) {
                    return i;
                }
            }
            return -1;
        }

        void Serialize(MIPS* mips) const override {
            mips->label(Names::FormDispTableName(name));
            for (const auto &item : methods) {
                mips->word(item.first + "." + item.second);
            }
        }
    private:
        std::string name;
        std::vector<std::pair<std::string, std::string>> methods;
    };
}