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
    context.EnterClass(*klass);
    BuildInit(*klass);
    if (!type_env_.class_table_.IsBasicClass(klass->type)) {
        for (auto feature: klass->features) {
            VisitFeature(&feature);
        }
    }
}

void backend::AsmModule::VisitMethodFeature(parser::MethodFeature *methodFeature) {
    context.EnterMethod(*methodFeature);

    mips->label("Main." + methodFeature->id.id)
        ->prolog(0);
    VisitExpression(&*methodFeature->expr);
    mips->epilog(0);
}

void backend::AsmModule::VisitAssignExpression(parser::AssignExpression *expr) {  }
void backend::AsmModule::VisitStaticDispatchExpression(parser::StaticDispatchExpression *expr) {  }

void backend::AsmModule::VisitDispatchExpression(parser::DispatchExpression *expr) {
    for (const auto &arg : expr->list) {
        VisitExpression(&*arg);
        mips->push(R::acc);
        context.AddLocalId("");
    }

    VisitExpression(&*expr->expr);

    auto dispatchLabel = NextLabel();
    mips->bne(R::acc, R::zero, dispatchLabel)
        ->la(R::acc, GetOrCreateConstFor(context.GetFileName()))
        ->li(R::t1, expr->line_number)
        ->jal("_dispatch_abort")
        ->label(dispatchLabel);

    auto target = expr->expr->result_type == semant::ClassTable::self_type ? context.GetSelfType() : expr->expr->result_type;
    auto offset = GetMethodOffset(target, expr->id->id);
    mips->lw(R::t1, R::acc.Shift(8))            // get dispatch table
        ->lw(R::t1, R::t1.Shift(offset * 4))    // get method address
        ->jalr(R::t1);
}

void backend::AsmModule::VisitIfExpression(parser::IfExpression *expr) {  }
void backend::AsmModule::VisitWhileExpression(parser::WhileExpression *expr) {  }
void backend::AsmModule::VisitBlockExpression(parser::BlockExpression *expr) {  }
void backend::AsmModule::VisitLetExpression(parser::LetExpression *expr) {  }
void backend::AsmModule::VisitCaseExpression(parser::CaseExpression *expr) {  }
void backend::AsmModule::VisitCaseBranchExpression(parser::CaseBranchExpression *expr) {  }
void backend::AsmModule::VisitNewExpression(parser::NewExpression *expr) {  }
void backend::AsmModule::VisitIsVoidExpression(parser::IsVoidExpression *expr) {  }
void backend::AsmModule::VisitPlusExpression(parser::PlusExpression *expr) {  }
void backend::AsmModule::VisitMinusExpression(parser::MinusExpression *expr) {  }
void backend::AsmModule::VisitMulExpression(parser::MulExpression *expr) {  }
void backend::AsmModule::VisitDivExpression(parser::DivExpression *expr) {  }
void backend::AsmModule::VisitInverseExpression(parser::InverseExpression *expr) {  }
void backend::AsmModule::VisitLessExpression(parser::LessExpression *expr) {  }
void backend::AsmModule::VisitLessOrEqualExpression(parser::LessOrEqualExpression *expr) {  }
void backend::AsmModule::VisitEqualExpression(parser::EqualExpression *expr) {  }
void backend::AsmModule::VisitNotExpression(parser::NotExpression *expr) {  }
void backend::AsmModule::VisitInBracketsExpression(parser::InBracketsExpression *expr) {  }

void backend::AsmModule::VisitIntExpression(parser::IntExpression *expr) {
    mips->la(R::acc, GetOrCreateConstFor(expr->value));
}

void backend::AsmModule::VisitStringExpression(parser::StringExpression *expr) {
    auto value_without_quotes = expr->value.substr(1, expr->value.size() - 2);
    mips->la(R::acc, GetOrCreateConstFor(value_without_quotes));
}

void backend::AsmModule::VisitBoolExpression(parser::BoolExpression *expr) {
    mips->la(R::acc, GetOrCreateConstFor(expr->value));
}

void backend::AsmModule::VisitIdExpression(parser::IdExpression *expr) {
    auto local_offset = context.GetPositionForLocal(expr->id);
    auto formal_offset = context.GetOffsetForFormal(expr->id);
    auto attr_offset = context.GetOffsetForAttr(expr->id);

    if (local_offset != -1) {
        mips->lw(R::acc, R::fp.Shift(4 * local_offset));
        return;
    }
    if (formal_offset != -1) {
        mips->lw(R::acc, R::fp.Shift(formal_offset));
        return;
    }
    if (attr_offset != -1) {
        mips->lw(R::acc, R::s0.Shift(attr_offset));
        return;
    }
    mips->move(R::acc, R::s0); // self
}

void backend::AsmModule::VisitNoExprExpression(parser::NoExprExpression *expr) {
    mips->move(R::acc, R::zero);
}
