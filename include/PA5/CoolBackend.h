#pragma once
#include "TypeEnvironment.h"

namespace backend {
    class CoolBackend {
    public:
        static std::string Convert(const semant::TypeEnvironment& env);
    };
}