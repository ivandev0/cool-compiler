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
            mips->global(name + "_protObj")
                ->word("-1")
                ->label(name + "_protObj")
                ->word(tag)
                ->word(types.size() + 3)
                ->word(name + "_dispTab");

            for (const auto &type : types) {
                if (type == "String" && name != "String") {
                    mips->word("str_const0");
                } else if (type == "Int" && name != "Int") {
                    mips->word("int_const0");
                } else if (type == "Bool" && name != "Bool") {
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

        void Serialize(MIPS* mips) const override {
            mips->label(name + "_dispTab");
            for (const auto &item : methods) {
                mips->word(item.first + "." + item.second);
            }
        }
    private:
        std::string name;
        std::vector<std::pair<std::string, std::string>> methods;
    };
}