#pragma once
#include <string>

namespace backend {
    class MIPS {
    public:
        MIPS* data() {
            data_ << "\t.data\t" << "\n";
            return this;
        }

        MIPS* text() {
            data_ << "\t.text\t" << "\n";
            return this;
        }

        MIPS* align(std::size_t align) {
            data_ << "\t.align\t" << align << "\n";
            return this;
        }

        MIPS* word(const std::string& str) {
            data_ << "\t.word\t" << str << "\n";
            return this;
        }

        MIPS* word(std::size_t value) {
            data_ << "\t.word\t" << value << "\n";
            return this;
        }

        MIPS* global(const std::string& str) {
            data_ << "\t.globl\t" << str << "\n";
            return this;
        }

        MIPS* label(const std::string& label) {
            data_ << label << ":\n";
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

        std::string End() {
            auto data = data_.str();
            data_.clear();
            return data;
        }

    private:
        std::stringstream data_;
    };

    class Serializable {
        virtual void Serialize(MIPS* mips) const = 0;
    };
}