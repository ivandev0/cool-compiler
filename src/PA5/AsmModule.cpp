#include "AsmModule.h"

void backend::AsmModule::VisitProgram(parser::Program *program) {
    auto defined_classes = type_env_.class_table_.GetDefinedClasses();
    for (auto klass: defined_classes) {
        VisitClass(&klass);
    }
}

void backend::AsmModule::VisitClass(parser::Class *klass) {
    GetOrCreateConstFor("");
    GetOrCreateConstFor((std::size_t) 0);
    GetOrCreateConstFor(false);
    GetOrCreateConstFor(true);

    BuildPrototype(klass->type);
    BuildDispatchTable(klass->type);
    // BuildInit()
    // for each method -> build
}

std::string backend::Prototype::ToData() const {
    std::stringstream result;
    result << "\t.globl\t" << name << "_protObj\n";
    result << "\t.word\t-1\n";
    result << name << "_protObj:\n";
    result << "\t.word\t" << tag << "\n";
    result << "\t.word\t" << types.size() + 3 << "\n";
    result << "\t.word\t" << name << "_dispTab\n";

    for (const auto &type : types) {
        std::string attr;
        if (type == "String" && name != "String") {
            attr = "str_const0";
        } else if (type == "Int" && name != "Int") {
            attr = "int_const0";
        } else if (type == "Bool" && name != "Bool") {
            attr = "bool_const0";
        } else {
            attr = "0";
        }
        result << "\t.word\t" << attr << "\n";
    }
    return result.str();
}

