#include "SemanticAnalyzer.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
void semant::SemanticAnalyzer::Analyze(parser::Program *program) {
    env_ = new TypeEnvironment(program->classes);
    CheckMain();
    VisitProgram(program);
}

std::string semant::SemanticAnalyzer::VisitProgram(parser::Program *program) {
    for (auto klass: program->classes) {
        VisitClass(&klass);
    }
    return program->result_type;
}

std::string semant::SemanticAnalyzer::VisitClass(parser::Class *klass) {
    env_->EnterClass(*klass);
    VisitParentAttrFeature(env_->class_table_.GetParentOf(klass->type));
    CheckAttributesAreUniqueFromParent(*klass);
    for (auto feature : klass->features) {
        if (auto data = std::get_if<parser::AttrFeature>(&feature.feature)) {
            VisitAttrFeature(data);
        }
    }
    CheckAttributesAreUnique(*klass);

    for (auto feature : klass->features) {
        if (auto data = std::get_if<parser::MethodFeature>(&feature.feature)) {
            VisitMethodFeature(data);
        }
    }
    env_->ExitClass();

    return klass->result_type;
}

void semant::SemanticAnalyzer::VisitParentAttrFeature(const std::string& parent) {
    if (parent == env_->class_table_.object_class_.type || parent == env_->class_table_.io_class_.type) return;
    VisitParentAttrFeature(env_->class_table_.GetParentOf(parent));

    auto attrs = env_->class_table_.GetAttributesOf(parent);
    for (auto& attr: attrs) {
        VisitAttrFeature(&attr);
    }
}

std::string semant::SemanticAnalyzer::VisitAttrFeature(parser::AttrFeature *attrFeature) {
    if (attrFeature->id.id == "self") {
        throw std::runtime_error("'self' cannot be the name of an attribute.");
    }

    env_->object_env_.AddIdWithType(attrFeature->id.id, attrFeature->type);

    if (ContainsNoExpr(attrFeature->expr)) {
        attrFeature->result_type = attrFeature->type;
        return attrFeature->result_type;
    }

    env_->object_env_.AddIdWithType("self", ClassTable::self_type);
    auto t1 = VisitExpression(&*attrFeature->expr);
    if (env_->CheckAIsSubtypeOfB(t1, attrFeature->type)) {
        attrFeature->result_type = attrFeature->type;
        env_->object_env_.PopId("self");
        return attrFeature->result_type;
    }

    std::string message;
    message.append("Inferred type ").append(attrFeature->expr->result_type)
        .append(" of attribute ").append(attrFeature->id.id)
        .append(" does not conform to declared type ").append(attrFeature->type).append(".");
    throw std::runtime_error(message);
}

std::string semant::SemanticAnalyzer::VisitMethodFeature(parser::MethodFeature *methodFeature) {
    env_->object_env_.EnterScope();

    env_->object_env_.AddIdWithType("self", ClassTable::self_type);
    for (const auto& param: methodFeature->params) {
        if (param.type == ClassTable::self_type) {
            throw std::runtime_error("Formal parameter " + param.id.id + " cannot have type SELF_TYPE.");
        }
        if (env_->object_env_.IsDefinedInLocalScope(param.id.id)) {
            throw std::runtime_error("Formal parameter " + param.id.id + " is multiply defined.");
        }
        env_->object_env_.AddIdWithType(param.id.id, param.type);
    }

    auto t0_ = VisitExpression(&*methodFeature->expr);
    if (!env_->CheckAIsSubtypeOfB(t0_, methodFeature->return_type)) {
        std::string message;
        message.append("Inferred return type ").append(t0_)
            .append(" of method ").append(methodFeature->id.id)
            .append(" does not conform to declared return type ").append(methodFeature->return_type).append(".");
        throw std::runtime_error(message);
    }

    env_->object_env_.ExitScope();
    return methodFeature->result_type;
}

std::string semant::SemanticAnalyzer::VisitFormal(parser::Formal *formal) {
    return formal->result_type;
}

std::string semant::SemanticAnalyzer::VisitAssignExpression(parser::AssignExpression *expr) {
    if (expr->id->id == "self") {
        throw std::runtime_error("Cannot assign to 'self'.");
    }
    if (!env_->object_env_.IsDefined(expr->id->id)) {
        throw std::runtime_error("Assignment to undeclared variable " + expr->id->id + ".");
    }

    auto t = env_->object_env_.GetType(expr->id->id);
    auto t_ = VisitExpression(&*expr->expr);
    if (!env_->CheckAIsSubtypeOfB(t_, t)) {
        std::string message;
        message.append("Type ").append(t_)
            .append(" of assigned expression does not conform to declared type ").append(t)
            .append(" of identifier ").append(expr->id->id).append(".");
        throw std::runtime_error(message);
    }
    expr->result_type = t_;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitStaticDispatchExpression(parser::StaticDispatchExpression *expr) {
    auto t0 = VisitExpression(&*expr->expr);
    if (env_->IsNotDefined(t0)) {
        throw std::runtime_error("Dispatch on undefined class " + t0 + ".");
    }

    std::vector<std::string> types;
    for (const auto& arg : expr->list) {
        types.push_back(VisitExpression(&*arg));
    }

    auto t = expr->type;
    if (!env_->CheckAIsSubtypeOfB(t0, t)) {
        std::string message;
        message.append("Expression type ").append(t0)
                .append(" does not conform to declared static dispatch type ").append(t).append(".");
        throw std::runtime_error(message);
    }

    auto signature = env_->method_env_.GetMethodSignature(t, expr->id->id);
    if (signature == nullptr) {
        throw std::runtime_error("Static dispatch to undefined method " + expr->id->id + ".");
    }
    if (signature->types.size() - 1 != types.size()) {
        throw std::runtime_error("Method " + expr->id->id + " called with wrong number of arguments.");
    }

    for (std::size_t i = 0; i < types.size(); ++i) {
        if (!env_->CheckAIsSubtypeOfB(types[i], signature->types[i])) {
            std::string message;
            message.append("In call of method ").append(expr->id->id)
                    .append(", type ").append(types[i])
                    .append(" of parameter ").append(signature->names[i])
                    .append(" does not conform to declared type ").append(signature->types[i]).append(".");
            throw std::runtime_error(message);
        }
    }

    expr->result_type = signature->types.back() == semant::ClassTable::self_type ? t0 : signature->types.back();
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitDispatchExpression(parser::DispatchExpression *expr) {
    auto t0 = VisitExpression(&*expr->expr);
    if (!env_->IsDefined(t0)) {
        throw std::runtime_error("Dispatch on undefined class " + t0 + ".");
    }

    std::vector<std::string> types;
    for (const auto& arg : expr->list) {
        types.push_back(VisitExpression(&*arg));
    }

    auto t0_ = t0 == ClassTable::self_type ? env_->self_type_ : t0;

    auto signature = env_->method_env_.GetMethodSignature(t0_, expr->id->id);
    if (signature == nullptr) {
        throw std::runtime_error("Dispatch to undefined method " + expr->id->id + ".");
    }
    if (signature->types.size() - 1 != types.size()) {
        throw std::runtime_error("Method " + expr->id->id + " called with wrong number of arguments.");
    }

    for (std::size_t i = 0; i < types.size(); ++i) {
        if (!env_->CheckAIsSubtypeOfB(types[i], signature->types[i])) {
            std::string message;
            message.append("In call of method ").append(expr->id->id)
                .append(", type ").append(types[i])
                .append(" of parameter ").append(signature->names[i])
                .append(" does not conform to declared type ").append(signature->types[i]).append(".");
            throw std::runtime_error(message);
        }
    }

    expr->result_type = signature->types.back() == semant::ClassTable::self_type ? t0 : signature->types.back();
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitIfExpression(parser::IfExpression *expr) {
    auto e1 = VisitExpression(&*expr->condition);
    if (e1 != env_->class_table_.bool_class_.type) {
        throw std::runtime_error("Predicate of 'if' does not have type Bool.");
    }

    auto t2 = VisitExpression(&*expr->trueBranch);
    auto t3 = VisitExpression(&*expr->falseBranch);
    std::vector<std::string> types{t2, t3};
    expr->result_type = env_->Combine(types);
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitWhileExpression(parser::WhileExpression *expr) {
    auto cond_type = VisitExpression(&*expr->condition);
    if (cond_type != env_->class_table_.bool_class_.type) {
        throw std::runtime_error("Loop condition does not have type Bool.");
    }

    VisitExpression(&*expr->body);

    expr->result_type = env_->class_table_.object_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitBlockExpression(parser::BlockExpression *expr) {
    std::string last_expr_type = env_->class_table_.object_class_.type;
    for (const auto& stmt: expr->list) {
        last_expr_type = VisitExpression(&*stmt);
    }
    expr->result_type = last_expr_type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitLetExpression(parser::LetExpression *expr) {
    env_->object_env_.EnterScope();

    if (expr->id->id == "self") {
        throw std::runtime_error("'self' cannot be bound in a 'let' expression.");
    }

    auto t0 = expr->type;
    if (env_->IsNotDefined(t0)) {
        throw std::runtime_error("Class " + t0 + " of let-bound identifier " + expr->id->id + " is undefined.");
    }

    auto t1 = VisitExpression(&*expr->expr);
    if (t1 != "no_type_" && !env_->CheckAIsSubtypeOfB(t1, t0)) {
        std::string message;
        message.append("Inferred type ").append(t1)
            .append(" of initialization of ").append(expr->id->id)
            .append(" does not conform to identifier's declared type ").append(t0).append(".");
        throw std::runtime_error(message);
    }

    env_->object_env_.AddIdWithType(expr->id->id, t0);
    expr->result_type = VisitExpression(&*expr->body);
    env_->object_env_.ExitScope();
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitCaseExpression(parser::CaseExpression *expr) {
    auto t0 = VisitExpression(&*expr->expr);

    std::vector<std::string> case_types;
    for (const auto& branch: expr->branches) {
        if (std::find(case_types.begin(), case_types.end(), branch.type) != case_types.end()) {
            throw std::runtime_error("Duplicate branch " + branch.type + " in case statement.");
        }
        case_types.push_back(branch.type);
    }

    std::vector<std::string> types;
    for (auto branch: expr->branches) {
        types.push_back(VisitCaseBranchExpression(&branch));
    }
    expr->result_type = env_->Combine(types);
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitNewExpression(parser::NewExpression *expr) {
    if(expr->type != ClassTable::self_type && !env_->IsDefined(expr->type)) {
        throw std::runtime_error("'new' used with undefined class " + expr->type + ".");
    }
    expr->result_type = expr->type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitIsVoidExpression(parser::IsVoidExpression *expr) {
    VisitExpression(&*expr->expr);
    expr->result_type = env_->class_table_.bool_class_.type;
    return expr->result_type;
}

template<class T>
std::string semant::SemanticAnalyzer::VisitBinaryExpression(T *expr, const std::string& op) {
    auto t1 = VisitExpression(&*expr->lhs);
    auto t2 = VisitExpression(&*expr->rhs);
    if (t1 != env_->class_table_.int_class_.type || t2 != env_->class_table_.int_class_.type) {
        throw std::runtime_error("non-Int arguments: " + t1 + " " + op + " " + t2);
    }

    expr->result_type = (op == "<" || op == "<=" ? env_->class_table_.bool_class_ : env_->class_table_.int_class_).type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitPlusExpression(parser::PlusExpression *expr) {
    return VisitBinaryExpression(expr, "+");
}

std::string semant::SemanticAnalyzer::VisitMinusExpression(parser::MinusExpression *expr) {
    return VisitBinaryExpression(expr, "-");
}

std::string semant::SemanticAnalyzer::VisitMulExpression(parser::MulExpression *expr) {
    return VisitBinaryExpression(expr, "*");
}

std::string semant::SemanticAnalyzer::VisitDivExpression(parser::DivExpression *expr) {
    return VisitBinaryExpression(expr, "/");
}

std::string semant::SemanticAnalyzer::VisitInverseExpression(parser::InverseExpression *expr) {
    auto type = VisitExpression(&*expr->expr);
    if (type != env_->class_table_.int_class_.type) {
        throw std::runtime_error("Argument of '~' has type " + type + " instead of Int.");
    }

    expr->result_type = env_->class_table_.int_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitLessExpression(parser::LessExpression *expr) {
    return VisitBinaryExpression(expr, "<");
}

std::string semant::SemanticAnalyzer::VisitLessOrEqualExpression(parser::LessOrEqualExpression *expr) {
    return VisitBinaryExpression(expr, "<=");
}

std::string semant::SemanticAnalyzer::VisitEqualExpression(parser::EqualExpression *expr) {
    auto t1 = VisitExpression(&*expr->lhs);
    auto t2 = VisitExpression(&*expr->rhs);

    if (env_->class_table_.IsBasicPrimitiveClass(t1) && env_->class_table_.IsBasicPrimitiveClass(t2) && t1 != t2) {
        throw std::runtime_error("Illegal comparison with a basic type.");
    }

    expr->result_type = env_->class_table_.bool_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitNotExpression(parser::NotExpression *expr) {
    auto type = VisitExpression(&*expr->expr);
    if (type != env_->class_table_.bool_class_.type) {
        throw std::runtime_error("Argument of 'not' has type " + type + " instead of Bool.");
    }

    expr->result_type = env_->class_table_.bool_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitInBracketsExpression(parser::InBracketsExpression *expr) {
    expr->result_type = VisitExpression(&*expr->expr);
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitIntExpression(parser::IntExpression *expr) {
    expr->result_type = env_->class_table_.int_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitStringExpression(parser::StringExpression *expr) {
    expr->result_type = env_->class_table_.str_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitBoolExpression(parser::BoolExpression *expr) {
    expr->result_type = env_->class_table_.bool_class_.type;
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitIdExpression(parser::IdExpression *expr) {
    if (expr->id == "self") {
        expr->result_type = ClassTable::self_type;
    } else {
        expr->result_type = env_->object_env_.GetType(expr->id);
    }
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitNoExprExpression(parser::NoExprExpression *expr) {
    return expr->result_type;
}

std::string semant::SemanticAnalyzer::VisitCaseBranchExpression(parser::CaseBranchExpression *expr) {
    env_->object_env_.EnterScope();
    env_->object_env_.AddIdWithType(expr->id->id, expr->type);
    expr->result_type = VisitExpression(&*expr->expr);
    env_->object_env_.ExitScope();
    return expr->result_type;
}

#pragma clang diagnostic pop