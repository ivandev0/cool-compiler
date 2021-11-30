#include "CoolBackend.h"
#include "AsmModule.h"

std::string backend::CoolBackend::Convert(const semant::TypeEnvironment& env) {
    AsmModule asm_module_(env);
    asm_module_.VisitProgram(nullptr);
    return asm_module_.ToData();
}
