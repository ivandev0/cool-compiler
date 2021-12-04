#pragma once
#include <string>
#include "Registers.h"

namespace backend {
    class MIPS {
    public:
        MIPS* data() {
            data_ << "\t.data\t" << "\n";
            return this;
        }

        MIPS* text() {
            heap_ << "\t.text\t" << "\n";
            return this;
        }

        MIPS* align(std::size_t align) {
            data_ << "\t.align\t" << align << "\n";
            return this;
        }

        MIPS* word(const std::string& str) {
            (heap_mode_ ? heap_ : data_) << "\t.word\t" << str << "\n";
            return this;
        }

        MIPS* word(std::size_t value) {
            (heap_mode_ ? heap_ : data_) << "\t.word\t" << value << "\n";
            return this;
        }

        MIPS* global(const std::string& str) {
            (heap_mode_ ? heap_ : data_) << "\t.globl\t" << str << "\n";
            return this;
        }

        MIPS* label(const std::string& label) {
            (heap_mode_ ? heap_ : data_) << label << ":\n";
            return this;
        }

        MIPS* ascii(const std::string& str) {
            data_ << "\t.ascii\t\"" << str << "\"\n";
            return this;
        }

        MIPS* byte(std::size_t size) {
            data_ << "\t.byte\t" << size << "\n";
            return this;
        }

        MIPS* prolog(int n) {
            addiu(R::sp, R::sp, -12 - n);
            sw(R::fp, R::sp.Shift(12));
            sw(R::s0, R::sp.Shift(8));
            sw(R::ra, R::sp.Shift(4));
            addiu(R::fp, R::sp, 4);
            move(R::s0, R::acc);
            return this;
        }

        MIPS* epilog(int n) {
            lw(R::fp, R::sp.Shift(12));
            lw(R::s0, R::sp.Shift(8));
            lw(R::ra, R::sp.Shift(4));
            addiu(R::sp, R::sp, 12 + n);
            jr(R::ra);
            return this;
        }

        MIPS* jal(const std::string& label) {
            heap_ << "\tjal\t" << label << "\n";
            return this;
        }

        MIPS* jr(const Reg& r) {
            heap_ << "\tjr\t" << r << "\n";
            return this;
        }

        MIPS* addiu(const Reg& r1, const Reg& r2, int arg) {
            heap_ << "\taddiu\t" << r1 << " " << r2 << " " << arg << "\n";
            return this;
        }

        MIPS* la(const Reg& r, const std::string& addr) {
            heap_ << "\tla\t" << r << " " << addr << "\n";
            return this;
        }

        MIPS* lw(const Reg& r1, const Reg& r2) {
            heap_ << "\tlw\t" << r1 << " " << r2 << "\n";
            return this;
        }

        MIPS* sw(const Reg& r1, const Reg& r2) {
            heap_ << "\tsw\t" << r1 << " " << r2 << "\n";
            return this;
        }

        MIPS* move(const Reg& to, const Reg& from) {
            heap_ << "\tmove\t" << to << " " << from << "\n";
            return this;
        }

        MIPS* push(const Reg& r) {
            sw(r, R::sp.Shift(0));
            addiu(R::sp, R::sp, -4);
            return this;
        }

        MIPS* pop(const Reg& r) {
            addiu(R::sp, R::sp, 4);
            lw(r, R::sp.Shift(0));
            return this;
        }

        MIPS* pop() {
            addiu(R::sp, R::sp, 4);
            return this;
        }

        void SetHeapMode() {
            heap_mode_ = true;
        }

        void SetDataMode() {
            heap_mode_ = false;
        }

        std::string End() {
            auto data = data_.str() + "\n" + heap_.str();
            data_.clear();
            heap_.clear();
            heap_mode_ = false;
            return data;
        }

    private:
        std::stringstream data_;
        std::stringstream heap_;
        bool heap_mode_ = false;
    };

    class Serializable {
        virtual void Serialize(MIPS* mips) const = 0;
    };
}