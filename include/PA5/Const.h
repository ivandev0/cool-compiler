#pragma once

#include <utility>
#include <cmath>
#include "MIPS.h"

namespace backend {
    template <class T>
    struct Const: public Serializable {
        virtual bool Match(T value) const = 0;
    };

    class IntConst: public Const<std::size_t> {
    public:
        IntConst(std::size_t val, std::size_t int_tag) : val(val), int_tag(int_tag) {};

        void Serialize(MIPS* mips) const override {
            mips->word("-1")
                ->label("int_const" + std::to_string(val))
                ->word(std::to_string(int_tag))
                ->word(4)
                ->word("Int_dispTab")
                ->word(val);
        }

        bool Match(size_t value) const override {
            return val == value;
        }
    private:
        std::size_t val;
        std::size_t int_tag;
    };

    class BoolConst: public Const<bool> {
    public:
        BoolConst(bool val, std::size_t bool_tag) : val(val), bool_tag(bool_tag) {};

        void Serialize(MIPS* mips) const override {
            mips->global("bool_const" + std::to_string(val))
                ->word("-1")
                ->label("bool_const" + std::to_string(val))
                ->word(std::to_string(bool_tag))
                ->word(4)
                ->word("Bool_dispTab")
                ->word(val);
        }

        bool Match(bool value) const override {
            return val == value;
        }
    private:
        bool val;
        std::size_t bool_tag;
    };

    class StrConst: public Const<std::string> {
    public:
        StrConst(std::string val, std::size_t str_tag, std::size_t index, std::string size_const) :
            val(std::move(val)), str_tag(str_tag), index(index), size_const(std::move(size_const)) {};

        void Serialize(MIPS* mips) const override {
            mips->word("-1")
                    ->label("str_const" + std::to_string(index))
                    ->word(std::to_string(str_tag));

            if (!val.empty()) {
                mips->word(4 + static_cast <int> (std::floor(val.size() / 4.0)));
            } else {
                mips->word(5);
            }

            mips->word("String_dispTab")->word(size_const);

            if (!val.empty()) {
                mips->ascii(val);
            }
            mips->byte(0)->align(2);
        }

        bool Match(std::string value) const override {
            return val == value;
        }
    private:
        std::string val;
        std::size_t str_tag;
        std::size_t index;
        std::string size_const;
    };
}