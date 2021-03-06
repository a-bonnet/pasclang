#include "Semantic/TypeChecker.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>

#include "Pasclang.h"

namespace pasclang::Semantic {

// Various error and warning reporting routines
void TypeChecker::wrongType(const TOT::Type* type, const TOT::Type* expected,
                            const Parsing::Position* start,
                            const Parsing::Position* end) const {
    std::ostringstream message;
    std::string lhs = type->kind == TOT::TypeKind::Boolean ? "bool" : "int";
    std::string rhs = expected->kind == TOT::TypeKind::Boolean ? "bool" : "int";
    message << "unexpected type " << lhs << "[" << type->dimension
            << "] instead of " << rhs << "[" << expected->dimension << "]";

    this->reporter->message(Message::MessageType::Error, message.str(), start,
                            end);
    this->errorHappened = true;
}

void TypeChecker::invalidCall(std::string& name, const Parsing::Position* start,
                              const Parsing::Position* end) const {
    std::ostringstream message;
    message << "invalid call to procedure or function " << name;

    this->reporter->message(Message::MessageType::Error, message.str(), start,
                            end);
    this->errorHappened = true;
}

void TypeChecker::invalidArity(const std::string& name,
                               const Parsing::Position* start,
                               const Parsing::Position* end) const {
    std::ostringstream message;
    message << "wrong number of arguments in call to " << name;

    this->reporter->message(Message::MessageType::Error, message.str(), start,
                            end);
    this->errorHappened = true;
}

void TypeChecker::undefinedSymbol(const std::string& symbol,
                                  const Parsing::Position* start,
                                  const Parsing::Position* end) const {
    std::ostringstream message;
    message << "undefined symbol " << symbol;

    this->reporter->message(Message::MessageType::Error, message.str(), start,
                            end);
    this->errorHappened = true;
}

void TypeChecker::redefiningSymbol(const std::string& symbol,
                                   const Parsing::Position* start,
                                   const Parsing::Position* end) const {
    std::ostringstream message;
    message << "redefinition of symbol " << symbol;

    this->reporter->message(Message::MessageType::Error, message.str(), start,
                            end);
    this->errorHappened = true;
}

void TypeChecker::invalidAssignment(const TOT::Type* type,
                                    const Parsing::Position* start,
                                    const Parsing::Position* end) const {
    std::ostringstream message;
    std::string op = type->kind == TOT::TypeKind::Boolean ? "bool" : "int";
    message << "invalid assignment to type " << op << "[" << type->dimension
            << "]";

    this->reporter->message(Message::MessageType::Error, message.str(), start,
                            end);
    this->errorHappened = true;
}

void TypeChecker::uninitializedValue(const std::string& symbol,
                                     const std::string& function,
                                     const Parsing::Position* start,
                                     const Parsing::Position* end) const {
    std::ostringstream message;
    std::string name = function != "" ? " in function " + function : "";
    message << "using unitialized variable " << symbol
            << (function != "" ? " in function " + function : "");

    this->reporter->message(Message::MessageType::Warning, message.str(), start,
                            end);
}

void TypeChecker::unusedValue(const std::string& symbol,
                              const std::string& function,
                              const Parsing::Position* start,
                              const Parsing::Position* end) const {
    std::ostringstream message;
    message << "unused variable " << symbol
            << (function != "" ? " in function " + function : "");

    this->reporter->message(Message::MessageType::Warning, message.str(), start,
                            end);
}

void TypeChecker::readDeclaration(const AST::Procedure* definition) const {
    std::string name = definition->getName();

    if (this->procedures.find(name) != this->procedures.end() ||
        this->globals.find(name) != this->globals.end())
        this->redefiningSymbol(name, &definition->getLocation()->getStart(),
                               nullptr);

    if (definition->getResultType() != nullptr) {
        this->procedures[name][name] = definition->getResultType();
    }

    auto formals = definition->getFormals();
    for (auto argument : formals) {
        if (this->locals.find(argument.first) == this->locals.end()) {
            this->procedures[name][argument.first] = argument.second;
        } else
            this->redefiningSymbol(argument.first,
                                   &argument.second->getLocation()->getStart(),
                                   nullptr);
    }
}

void TypeChecker::check(std::unique_ptr<AST::Program>& ast) {
    this->errorHappened = false;
    this->globals.clear();
    this->procedures.clear();
    this->locals.clear();

    std::swap(ast, this->ast);

    this->booleanType = this->ast->getTypes().get(TOT::TypeKind::Boolean, 0);
    this->integerType = this->ast->getTypes().get(TOT::TypeKind::Integer, 0);

    this->ast->accept(*this);

    std::swap(ast, this->ast);

    if (this->errorHappened)
        throw PasclangException(ExitCode::TypeError);
}

void TypeChecker::visit(const AST::PrimitiveType& type) const {
    std::unique_ptr<Parsing::Location> location;
    this->lastType = type.getType();
}

void TypeChecker::visit(const AST::Expression&) const {}
void TypeChecker::visit(const AST::EConstant&) const {}

void TypeChecker::visit(const AST::ECBoolean&) const {
    this->lastType = booleanType;
}

void TypeChecker::visit(const AST::ECInteger&) const {
    this->lastType = integerType;
}

// returns type of the accessed variable if defined, marks it as used
void TypeChecker::visit(const AST::EVariableAccess& variable) const {
    if (this->locals.find(variable.getName()) != this->locals.end()) {
        this->lastType = this->locals[variable.getName()]->getType();
        this->localUsage[variable.getName()] = true;
    } else if (this->globals.find(variable.getName()) != this->globals.end()) {
        this->lastType = this->globals[variable.getName()]->getType();
        this->globalUsage[variable.getName()] = true;
    } else
        this->undefinedSymbol(variable.getName(),
                              &variable.getLocation()->getStart(),
                              &variable.getLocation()->getEnd());
}

// returns type of the unary operation if correct
void TypeChecker::visit(const AST::EUnaryOperation& operation) const {
    operation.getExpression().accept(*this);

    if (operation.getType() == AST::EUnaryOperation::Type::UnaryNot &&
        this->lastType != booleanType)
        this->wrongType(this->lastType, booleanType,
                        &operation.getExpression().getLocation()->getStart(),
                        &operation.getExpression().getLocation()->getEnd());

    else if (operation.getType() == AST::EUnaryOperation::Type::UnaryMinus &&
             this->lastType != integerType)
        this->wrongType(this->lastType, integerType,
                        &operation.getExpression().getLocation()->getStart(),
                        &operation.getExpression().getLocation()->getEnd());
}

// returns type of the binary operation if correct
void TypeChecker::visit(const AST::EBinaryOperation& operation) const {
    operation.getLeft().accept(*this);
    const TOT::Type* lhs = this->lastType;
    operation.getRight().accept(*this);

    switch (operation.getType()) {
    case AST::EBinaryOperation::Type::BinaryAddition:
    case AST::EBinaryOperation::Type::BinarySubtraction:
    case AST::EBinaryOperation::Type::BinaryMultiplication:
    case AST::EBinaryOperation::Type::BinaryDivision:
        if (lhs != integerType)
            this->wrongType(lhs, integerType,
                            &operation.getLeft().getLocation()->getStart(),
                            &operation.getLeft().getLocation()->getEnd());
        if (lhs != this->lastType)
            this->wrongType(this->lastType, lhs,
                            &operation.getRight().getLocation()->getEnd(),
                            &operation.getRight().getLocation()->getEnd());
        this->lastType = integerType;
        break;

    case AST::EBinaryOperation::Type::BinaryLogicalLessThan:
    case AST::EBinaryOperation::Type::BinaryLogicalLessEqual:
    case AST::EBinaryOperation::Type::BinaryLogicalGreaterThan:
    case AST::EBinaryOperation::Type::BinaryLogicalGreaterEqual:
        if (lhs != integerType)
            this->wrongType(lhs, integerType,
                            &operation.getLeft().getLocation()->getStart(),
                            &operation.getLeft().getLocation()->getEnd());
        if (lhs != this->lastType)
            this->wrongType(this->lastType, lhs,
                            &operation.getRight().getLocation()->getEnd(),
                            &operation.getRight().getLocation()->getEnd());
        this->lastType = booleanType;
        break;

    case AST::EBinaryOperation::Type::BinaryLogicalOr:
    case AST::EBinaryOperation::Type::BinaryLogicalAnd:
        if (lhs != booleanType)
            this->wrongType(lhs, booleanType,
                            &operation.getLeft().getLocation()->getStart(),
                            &operation.getLeft().getLocation()->getEnd());
        if (lhs != this->lastType)
            this->wrongType(this->lastType, lhs,
                            &operation.getRight().getLocation()->getStart(),
                            &operation.getRight().getLocation()->getEnd());
        this->lastType = booleanType;
        break;

    default:
        if (lhs != this->lastType)
            this->wrongType(lhs, this->lastType,
                            &operation.getRight().getLocation()->getStart(),
                            &operation.getRight().getLocation()->getEnd());
        this->lastType = booleanType;
        break;
    }
}

// basically like IProcedureCall but with return type
// checks actuals for expression type and returns function type if correct
void TypeChecker::visit(const AST::EFunctionCall& call) const {
    // built-in function
    if (call.getName() == "writeln" || call.getName() == "write") {
        std::string name = call.getName();
        this->invalidCall(name, &call.getLocation()->getStart(),
                          &call.getLocation()->getEnd());
    }

    if (call.getName() == "readln") {
        if (call.getActuals().size() != 0)
            this->invalidArity(call.getName(), &call.getLocation()->getStart(),
                               &call.getLocation()->getEnd());

        this->lastType = integerType;
        return;
    }

    std::string name = call.getName();
    if (this->procedures.find(name) == this->procedures.end()) {
        this->undefinedSymbol(name, &call.getLocation()->getStart(),
                              &call.getLocation()->getEnd());
        return;
    }

    if (this->procedures[name].find(name) == this->procedures[name].end()) {
        this->invalidCall(name, &call.getLocation()->getStart(),
                          &call.getLocation()->getEnd());
        return;
    }

    // Is valid since we made sure the function exists above

    const AST::Procedure* procedure;
    for (auto p : this->ast->getProcedures()) {
        if (p->getName() == name)
            procedure = p;
    }

    const auto formals = procedure->getFormals();
    const auto actuals = call.getActuals();

    if (formals.size() != actuals.size()) {
        this->invalidArity(name, &call.getLocation()->getStart(),
                           &call.getLocation()->getEnd());
        return;
    }

    auto formal = formals.begin();
    for (auto actual = actuals.begin(); actual != actuals.end();
         formal++, actual++) {
        (*actual)->accept(*this);
        if (formal->second->getType() != this->lastType)
            this->wrongType(this->lastType, formal->second->getType(),
                            &(*actual)->getLocation()->getStart(),
                            &(*actual)->getLocation()->getEnd());
    }

    if (procedure->getResultType()->getType() == nullptr)
        this->invalidCall(name, &call.getLocation()->getStart(),
                          &call.getLocation()->getEnd());

    this->lastType = procedure->getResultType()->getType();
}

// checks index for int type and returns the value at accessed address' type
void TypeChecker::visit(const AST::EArrayAccess& access) const {
    access.getIndex().accept(*this);
    if (this->lastType != integerType)
        this->wrongType(this->lastType, integerType,
                        &access.getIndex().getLocation()->getStart(),
                        &access.getIndex().getLocation()->getEnd());

    access.getArray().accept(*this);
    this->lastType = this->ast->getTypes().get(this->lastType->kind,
                                               this->lastType->dimension - 1);
}

// checks size expression for integer type and returns allocated array type
void TypeChecker::visit(const AST::EArrayAllocation& allocation) const {
    allocation.getElements().accept(*this);
    if (this->lastType != integerType) {
        this->wrongType(this->lastType, integerType,
                        &allocation.getElements().getLocation()->getStart(),
                        &allocation.getElements().getLocation()->getEnd());
    }

    allocation.getType().accept(*this);
}

void TypeChecker::visit(const AST::Instruction&) const {}

// basically like EFunctionCall but without return type
// checks arguments for correct type and makes sure callee has no return type
void TypeChecker::visit(const AST::IProcedureCall& call) const {
    // Built-in procedure
    if (call.getName() == "writeln" || call.getName() == "write") {
        if (call.getActuals().size() != 1)
            this->invalidArity(call.getName(), &call.getLocation()->getStart(),
                               &call.getLocation()->getEnd());

        call.getActuals().front()->accept(*this);

        if (this->lastType != integerType)
            this->wrongType(
                this->lastType, integerType,
                &call.getActuals().front()->getLocation()->getStart(),
                &call.getActuals().front()->getLocation()->getEnd());

        this->lastType = nullptr;
        return;
    }

    std::string name = call.getName();
    auto procedures = this->ast->getProcedures();
    auto p = std::find_if(procedures.begin(), procedures.end(),
                          [&name](const AST::Procedure* currentProcedure) {
                              return currentProcedure->getName() == name;
                          });

    if (p == this->ast->getProcedures().end())
        this->undefinedSymbol(name, &call.getLocation()->getStart(),
                              &call.getLocation()->getEnd());

    auto formals = (*p)->getFormals();

    const auto& actuals = call.getActuals();

    if (formals.size() != actuals.size())
        this->invalidArity(name, &actuals.front()->getLocation()->getStart(),
                           &actuals.back()->getLocation()->getEnd());

    auto formal = formals.begin();
    for (auto actual = actuals.begin(); actual != actuals.end();
         formal++, actual++) {
        (*actual)->accept(*this);
        if (formal->second->getType() != this->lastType)
            this->wrongType(this->lastType, formal->second->getType(),
                            &(*actual)->getLocation()->getStart(),
                            &(*actual)->getLocation()->getEnd());
    }

    if ((*p)->getResultType() != nullptr)
        this->invalidCall(name, &call.getLocation()->getStart(),
                          &call.getLocation()->getEnd());

    this->lastType = nullptr;
}

// checks expression type and variable type and marks variable as initialized
void TypeChecker::visit(const AST::IVariableAssignment& assignment) const {
    const AST::PrimitiveType* type = nullptr;
    std::string name = assignment.getName();

    if (locals.find(name) != locals.end()) {
        type = locals[name];
        this->localInitialized[name] = true;
    } else if (globals.find(name) != globals.end()) {
        type = globals[name];
        this->globalInitialized[name] = true;
    } else {
        assignment.getValue().accept(*this);
        this->undefinedSymbol(name, &assignment.getLocation()->getStart(),
                              &assignment.getValue().getLocation()->getStart());
    }
    assignment.getValue().accept(*this);

    if (type == nullptr) {
        return;
    } else if (this->lastType != type->getType()) {
        this->wrongType(this->lastType, type->getType(),
                        &assignment.getValue().getLocation()->getStart(),
                        &assignment.getValue().getLocation()->getEnd());
    }
}

// checks array dimension and index integer type and returns indexed expression
// type
void TypeChecker::visit(const AST::IArrayAssignment& assignment) const {
    const AST::EArrayAccess& access =
        static_cast<const AST::EArrayAccess&>(assignment.getArray());

    access.getIndex().accept(*this);
    const auto* indexType = this->lastType;
    if (indexType != integerType)
        this->wrongType(this->lastType, integerType,
                        &access.getIndex().getLocation()->getStart(),
                        &access.getIndex().getLocation()->getEnd());

    access.getArray().accept(*this);
    const auto* arrayType = this->ast->getTypes().get(
        this->lastType->kind, this->lastType->dimension - 1);

    assignment.getValue().accept(*this);
    const TOT::Type* valueType = this->lastType;
    if (arrayType != valueType)
        this->wrongType(arrayType, valueType,
                        &assignment.getValue().getLocation()->getStart(),
                        &assignment.getValue().getLocation()->getEnd());
}

// checks each instruction in sequence
void TypeChecker::visit(const AST::ISequence& sequence) const {
    for (const auto& instruction : sequence.getInstructions())
        instruction->accept(*this);
}

// checks condition for boolean type and checks instructions
void TypeChecker::visit(const AST::ICondition& condition) const {
    condition.getCondition().accept(*this);
    if (this->lastType != booleanType)
        this->wrongType(this->lastType, booleanType,
                        &condition.getCondition().getLocation()->getStart(),
                        &condition.getCondition().getLocation()->getStart());

    condition.getTrue().accept(*this);

    if (condition.getFalse() != nullptr)
        condition.getFalse()->accept(*this);
}

// checks condition for boolean type and checks instruction
void TypeChecker::visit(const AST::IRepetition& repetition) const {
    repetition.getCondition().accept(*this);
    if (this->lastType != booleanType)
        this->wrongType(this->lastType, booleanType, nullptr, nullptr);

    repetition.getInstructions().accept(*this);
}

// defines formals, locals and result type/variable and checks body
void TypeChecker::visit(const AST::Procedure& definition) const {
    this->localUsage.clear();
    this->localInitialized.clear();
    this->locals.clear();
    std::string name = definition.getName();
    this->currentFunction = name;

    if (definition.getResultType() != nullptr) {
        this->locals[name] = definition.getResultType();
        this->localUsage[name] = false;
        this->localInitialized[name] = false;
    }

    for (const auto& argument : definition.getFormals()) {
        if (this->locals.find(argument.first) == this->locals.end()) {
            this->locals[argument.first] = argument.second;
            this->localInitialized[argument.first] = true;
        } else
            this->redefiningSymbol(argument.first,
                                   &argument.second->getLocation()->getStart(),
                                   nullptr);
    }

    for (const auto& local : definition.getLocals()) {
        if (this->locals.find(local.first) == this->locals.end()) {
            this->locals[local.first] = local.second;
            this->localInitialized[local.first] = false;
            this->localUsage[local.first] = false;
        } else
            this->redefiningSymbol(
                local.first, &local.second->getLocation()->getStart(), nullptr);
    }

    definition.getBody().accept(*this);

    for (const auto& local : this->locals) {
        // Skip function name, for example functions that return a constant
        // value
        if (local.first == definition.getName())
            continue;
        if (!this->localUsage[local.first])
            this->unusedValue(local.first, definition.getName(), nullptr,
                              nullptr);
    }
}

// defines and checks globals and procedures and checks body
void TypeChecker::visit(const AST::Program& program) const {
    this->globalInitialized.clear();
    this->globalUsage.clear();

    // Table of global variables
    for (auto& global : program.getGlobals()) {
        if (this->globals.find(global.first) == this->globals.end()) {
            this->globals[global.first] = global.second;
            this->globalUsage[global.first] = false;
            this->globalInitialized[global.first] = false;
        } else
            this->redefiningSymbol(global.first,
                                   &global.second->getLocation()->getStart(),
                                   &global.second->getLocation()->getEnd());
    }

    // Checking declarations comes first since functions might call each other
    // recursively
    for (const auto& procedure : program.getProcedures())
        this->readDeclaration(procedure);

    // Table of functions and procedures
    for (auto& procedure : program.getProcedures())
        procedure->accept(*this);

    this->locals.clear();
    this->currentFunction = "";
    program.getMain().accept(*this);

    for (auto& global : this->globals) {
        if (!this->globalUsage[global.first])
            this->unusedValue(global.first, this->currentFunction, nullptr,
                              nullptr);
    }
}

} // namespace pasclang::Semantic
