#include <utility>
#include <cmath>

#pragma once

namespace backend {
    template <class T>
    struct Const {
        virtual std::string ToData() const = 0;
        virtual bool Match(T value) const = 0;
    };

    class IntConst: public Const<std::size_t> {
    public:
        IntConst(std::size_t val, std::size_t int_tag) : val(val), int_tag(int_tag) {};

        std::string ToData() const override {
            std::stringstream result;
            result << "\t.word\t-1\n";
            result << "int_const" << val << ":\n";
            result << "\t.word\t" << int_tag << "\n";
            result << "\t.word\t4\n";
            result << "\t.word\tInt_dispTab\n";
            result << "\t.word\t" << val << "\n";
            return result.str();
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

        std::string ToData() const override {
            std::stringstream result;
            result << "\t.globl\tbool_const" << val << "\n";
            result << "\t.word\t-1\n";
            result << "bool_const" << val << ":\n";
            result << "\t.word\t" << bool_tag << "\n";
            result << "\t.word\t4\n";
            result << "\t.word\tBool_dispTab\n";
            result << "\t.word\t" << val << "\n";
            return result.str();
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

        std::string ToData() const override {
            std::stringstream result;
            result << "\t.word\t-1\n";
            result << "str_const" << index << ":\n";
            result << "\t.word\t" << str_tag << "\n";

            if (!val.empty()) {
                result << "\t.word\t" << 4 + static_cast <int> (std::floor(val.size() / 4.0)) << "\n";
            } else {
                result << "\t.word\t5\n";
            }

            result << "\t.word\tString_dispTab\n";
            result << "\t.word\t" << size_const << "\n";

            if (!val.empty()) {
                result << "\t.ascii\t\"" << val << "\"\n";
            }
            result << "\t.byte\t0\t\n";
            result << "\t.align\t2";
            return result.str();
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