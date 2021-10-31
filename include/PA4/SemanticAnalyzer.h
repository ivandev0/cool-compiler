#pragma once
#include "Nodes.h"

namespace semant {
    class SemanticAnalyzer {
    public:
        parser::Program* Analyze(parser::Program* program);
    };
}
