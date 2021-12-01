#include "AsmModule.h"

void backend::AsmModule::VisitProgram(parser::Program *program) {
    GetOrCreateConstFor((std::string) "");
    GetOrCreateConstFor((std::size_t) 0);
    GetOrCreateConstFor(false);
    GetOrCreateConstFor(true);

    auto defined_classes = type_env_.class_table_.GetDefinedClasses();
    for (const auto &item : defined_classes) {
        GetOrCreateConstFor(item.type);
    }
    for (auto klass: defined_classes) {
        VisitClass(&klass);
    }
}

void backend::AsmModule::VisitClass(parser::Class *klass) {
    BuildPrototype(klass->type);
    BuildDispatchTable(klass->type);
    mips->SetHeapMode();
    BuildInit(*klass);
    if (!type_env_.class_table_.IsBasicClass(klass->type)) {
        for (auto feature: klass->features) {
            VisitFeature(&feature);
        }
    }
}

void backend::AsmModule::VisitAttrFeature(parser::AttrFeature *attrFeature) {
    // nothing
}

void backend::AsmModule::VisitMethodFeature(parser::MethodFeature *methodFeature) {
    mips->label("Main." + methodFeature->id.id)->prolog(0);
    // TODO
    mips->la("$a0", "int_const0");
    mips->epilog(0);
}
