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

    mips->label(context.GetSelfType() + "." + methodFeature->id.id)
        ->prolog(0);
    VisitExpression(&*methodFeature->expr);
    mips->epilog(methodFeature->params.size());
}

void backend::AsmModule::VisitAssignExpression(parser::AssignExpression *expr) {
    VisitExpression(&*expr->expr);

    auto local_offset = context.GetOffsetForLocal(expr->id->id);
    auto formal_offset = context.GetOffsetForFormal(expr->id->id);
    auto attr_offset = context.GetOffsetForAttr(expr->id->id);

    if (local_offset != -1) {
        mips->sw(R::acc, R::fp.Shift(local_offset));
        return;
    }
    if (formal_offset != -1) {
        mips->sw(R::acc, R::fp.Shift(formal_offset));
        return;
    }
    if (attr_offset != -1) {
        mips->sw(R::acc, R::s0.Shift(attr_offset))->genGc();
        return;
    }
}

void backend::AsmModule::VisitStaticDispatchExpression(parser::StaticDispatchExpression *expr) {
    VisitCommonDispatchExpression(&*expr->expr, expr->id->id, expr->type, expr->list);
}

void backend::AsmModule::VisitDispatchExpression(parser::DispatchExpression *expr) {
    auto target = expr->expr->result_type == semant::ClassTable::self_type ? context.GetSelfType() : expr->expr->result_type;
    VisitCommonDispatchExpression(&*expr->expr, expr->id->id, target, expr->list);
}

void backend::AsmModule::VisitCommonDispatchExpression(
    parser::Expression* expr,
    const std::string& name,
    const std::string& type,
    const std::vector<std::shared_ptr<parser::Expression>>& args
) {
    for (const auto &arg : args) {
        VisitExpression(&*arg);
        mips->push(R::acc);
        context.AddLocalId("");
    }

    VisitExpression(&*expr);

    auto dispatchLabel = NextLabel();
    mips->bne(R::acc, R::zero, dispatchLabel)
        ->la(R::acc, GetOrCreateConstFor(context.GetFileName()))
        ->li(R::t1, expr->line_number)
        ->jal(Names::dispatch_abort)
        ->label(dispatchLabel);

    auto offset = GetMethodOffset(type, name);
    mips->lw(R::t1, R::acc.Shift(8))            // get dispatch table
        ->lw(R::t1, R::t1.Shift(offset * 4))    // get method address
        ->jalr(R::t1);
}

void backend::AsmModule::VisitIfExpression(parser::IfExpression *expr) {
    auto falseBranch = NextLabel();
    auto end = NextLabel();

    VisitExpression(&*expr->condition);
    mips->lw(R::t1, R::acc.Shift(12))
        ->beqz(R::t1, falseBranch);
    VisitExpression(&*expr->trueBranch);
    mips->b(end)->label(falseBranch);
    VisitExpression(&*expr->falseBranch);
    mips->label(end);
}

void backend::AsmModule::VisitWhileExpression(parser::WhileExpression *expr) {  }

void backend::AsmModule::VisitBlockExpression(parser::BlockExpression *expr) {
    for (const auto &item : expr->list) {
        VisitExpression(&*item);
    }
}

void backend::AsmModule::VisitLetExpression(parser::LetExpression *expr) {
    InitVariable(&*expr->expr, expr->type);
    context.AddLocalId(expr->id->id);
    mips->push(R::acc);
    VisitExpression(&*expr->body);
    mips->pop();
    context.PopLocalId();
}

void backend::AsmModule::VisitCaseExpression(parser::CaseExpression *expr) {  }
void backend::AsmModule::VisitCaseBranchExpression(parser::CaseBranchExpression *expr) {  }

void backend::AsmModule::VisitNewExpression(parser::NewExpression *expr) {
    auto type = expr->result_type;
    if (type != semant::ClassTable::self_type) {
        mips->la(R::acc, Names::FormProtObjName(type))
            ->jal(Names::copy)
            ->jal(Names::FormInit(type));
        return;
    }
    mips->la(R::t1, Names::obj_tab)
        ->lw(R::t2, R::s0.Shift(0))     // load tag
        ->sll(R::t2, R::t2, 3)
        ->addu(R::t1, R::t1, R::t2)     // get reference to protObj
        ->move(R::t5, R::t1)            // use t5 as temporary because t0-t4 can be modified by runtime
        ->lw(R::acc, R::t1.Shift(0))    // load protObj to acc
        ->jal(Names::copy)
        ->lw(R::t1, R::t5.Shift(4))     // call init
        ->jalr(R::t1);
}

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

void backend::AsmModule::VisitInBracketsExpression(parser::InBracketsExpression *expr) {
    VisitExpression(&*expr->expr);
}

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
    auto local_offset = context.GetOffsetForLocal(expr->id);
    auto formal_offset = context.GetOffsetForFormal(expr->id);
    auto attr_offset = context.GetOffsetForAttr(expr->id);

    if (local_offset != -1) {
        mips->lw(R::acc, R::fp.Shift(local_offset));
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

void backend::AsmModule::InitVariable(parser::Expression *expr, const std::string& type) {
    if (std::get_if<parser::NoExprExpression>(&(expr->data)) != nullptr) {
        if (type == Names::int_name) mips->la(R::acc, GetOrCreateConstFor((std::size_t) 0));
        else if (type == Names::bool_name) mips->la(R::acc, GetOrCreateConstFor(false));
        else if (type == Names::str_name) mips->la(R::acc, GetOrCreateConstFor((std::string) ""));
        else VisitExpression(expr);;
    } else {
        VisitExpression(expr);
    }
}
