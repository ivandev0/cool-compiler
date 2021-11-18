#pragma once
#include "Parser.h"

namespace backend {
    class CoolBackend {
    public:
        explicit CoolBackend(std::basic_ostream<char> &stream) : stream(stream) {}
        void Convert(const parser::Program& program);

    private:
        std::ostream& stream;
    };
}