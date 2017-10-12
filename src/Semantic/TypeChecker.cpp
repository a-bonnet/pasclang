#include "Semantic/TypeChecker.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <memory>

#include "Pasclang.h"

namespace pasclang::Semantic {

// Various error and warning reporting routines
void TypeChecker::wrongType(const TOT::Type* type, const TOT::Type* expected, const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    std::string lhs = type->kind == TOT::TypeKind::Boolean ? "bool" : "int";
    std::string rhs = expected->kind == TOT::TypeKind::Boolean ? "bool" : "int";
    message << "unexpected type " << lhs << "[" << type->dimension  << "] instead of " << rhs << "[" << expected->dimension << "]";

    this->reporter->message(Message::MessageType::Error, message.str(), start, end);
    this->errorHappened = true;
}

void TypeChecker::invalidCall(std::string& name, const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    message << "invalid call to procedure or function " << name;

    this->reporter->message(Message::MessageType::Error, message.str(), start, end);
    this->errorHappened = true;
}

void TypeChecker::invalidArity(std::string& name, const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    message << "wrong number of arguments in call to " << name;

    this->reporter->message(Message::MessageType::Error, message.str(), start, end);
    this->errorHappened = true;
}

void TypeChecker::undefinedSymbol(const std::string& symbol, const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    message << "undefined symbol " << symbol;
    
    this->reporter->message(Message::MessageType::Error, message.str(), start, end);
    this->errorHappened = true;
}

void TypeChecker::redefiningSymbol(const std::string& symbol, const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    message << "redefinition of symbol " << symbol;
    
    this->reporter->message(Message::MessageType::Error, message.str(), start, end);
    this->errorHappened = true;
}

void TypeChecker::invalidAssignment(const TOT::Type* type, const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    std::string op = type->kind == TOT::TypeKind::Boolean ? "bool" : "int";
    message << "invalid assignment to type " << op << "[" << type->dimension << "]";

    this->reporter->message(Message::MessageType::Error, message.str(), start, end);
    this->errorHappened = true;
}

void TypeChecker::uninitializedValue(const std::string& symbol, const std::string& function,
        const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    std::string name = function != "" ? " in function " + function : "";
    message << "using unitialized variable " << symbol 
        << (function != "" ? " in function " + function : "");

    this->reporter->message(Message::MessageType::Warning, message.str(), start, end);
}

void TypeChecker::unusedValue(const std::string& symbol, const std::string& function,
        const Parsing::Position* start, const Parsing::Position* end)
{
    std::ostringstream message;
    message << "unused variable " << symbol << (function != "" ? " in function " + function : "");

    this->reporter->message(Message::MessageType::Warning, message.str(), start, end);
}

void TypeChecker::readDeclaration(AST::Procedure* definition)
{
    std::string name = definition->getName();

    if(this->procedures.find(name) != this->procedures.end() ||
        this->globals.find(name) != this->globals.end())
        this->redefiningSymbol(name, &definition->getLocation()->getStart(), nullptr);


    if(definition->getResultType().get() != nullptr)
    {
        this->procedures[name][name] = definition->getResultType().get();
    }

    for(auto& argument : definition->getFormals())
    {
        if(this->locals.find(argument.first) == this->locals.end())
        {
            this->procedures[name][argument.first] = argument.second.get();
        }
        else
            this->redefiningSymbol(argument.first, &argument.second->getLocation()->getStart(), nullptr);
    }
}

void TypeChecker::check(std::unique_ptr<AST::Program>& ast)
{
    this->errorHappened = false;
    this->globals.clear();
    this->procedures.clear();
    this->locals.clear();

    std::swap(ast, this->ast);

    this->booleanType = this->ast->getTypes()->get(TOT::TypeKind::Boolean, 0);
    this->integerType = this->ast->getTypes()->get(TOT::TypeKind::Integer, 0);

    this->ast->accept(*this);

    std::swap(ast, this->ast);

    if(this->errorHappened)
        throw PasclangException(ExitCode::TypeError);
}

void TypeChecker::visit(AST::PrimitiveType& type)
{
    std::unique_ptr<Parsing::Location> location;
    this->lastType = type.getType();
}

void TypeChecker::visit(AST::Expression&) { }
void TypeChecker::visit(AST::EConstant&) { }

void TypeChecker::visit(AST::ECBoolean&)
{
    this->lastType = booleanType;
}

void TypeChecker::visit(AST::ECInteger&)
{
    this->lastType = integerType;
}

// returns type of the accessed variable if defined, marks it as used
void TypeChecker::visit(AST::EVariableAccess& variable)
{
    if(this->locals.find(variable.getName()) != this->locals.end())
    {
        this->lastType = this->locals[variable.getName()]->getType();
        this->localUsage[variable.getName()] = true;
    }
    else if(this->globals.find(variable.getName()) != this->globals.end())
    {
        this->lastType = this->globals[variable.getName()]->getType();
        this->globalUsage[variable.getName()] = true;
    }
    else
        this->undefinedSymbol(variable.getName(),
                &variable.getLocation()->getStart(), &variable.getLocation()->getEnd());
}

// returns type of the unary operation if correct
void TypeChecker::visit(AST::EUnaryOperation& operation)
{
    operation.getExpression()->accept(*this);

    if(operation.getType() == AST::EUnaryOperation::UnaryNot
        && this->lastType != booleanType)
        this->wrongType(this->lastType, booleanType,
                &operation.getExpression()->getLocation()->getStart(),
                &operation.getExpression()->getLocation()->getEnd());

    else if(operation.getType() == AST::EUnaryOperation::UnaryMinus
        && this->lastType != integerType)
        this->wrongType(this->lastType, integerType,
                &operation.getExpression()->getLocation()->getStart(),
                &operation.getExpression()->getLocation()->getEnd());
}

// returns type of the binary operation if correct
void TypeChecker::visit(AST::EBinaryOperation& operation)
{
    operation.getLeft()->accept(*this);
    const TOT::Type* lhs = this->lastType;
    operation.getRight()->accept(*this);

    switch(operation.getType())
    {
        case AST::EBinaryOperation::BinaryAddition:
        case AST::EBinaryOperation::BinarySubtraction:
        case AST::EBinaryOperation::BinaryMultiplication:
        case AST::EBinaryOperation::BinaryDivision:
            if(lhs != integerType)
                this->wrongType(lhs, integerType,
                        &operation.getLeft()->getLocation()->getStart(),
                        &operation.getLeft()->getLocation()->getEnd());
            if(lhs != this->lastType)
                this->wrongType(this->lastType, lhs,
                        &operation.getRight()->getLocation()->getEnd(),
                        &operation.getRight()->getLocation()->getEnd());
            this->lastType = integerType;
            break;

        case AST::EBinaryOperation::BinaryLogicalLessThan:
        case AST::EBinaryOperation::BinaryLogicalLessEqual:
        case AST::EBinaryOperation::BinaryLogicalGreaterThan:
        case AST::EBinaryOperation::BinaryLogicalGreaterEqual:
            if(lhs != integerType)
                this->wrongType(lhs, integerType,
                        &operation.getLeft()->getLocation()->getStart(),
                        &operation.getLeft()->getLocation()->getEnd());
            if(lhs != this->lastType)
                this->wrongType(this->lastType, lhs,
                        &operation.getRight()->getLocation()->getEnd(),
                        &operation.getRight()->getLocation()->getEnd());
            this->lastType = booleanType;
            break;

        case AST::EBinaryOperation::BinaryLogicalOr:
        case AST::EBinaryOperation::BinaryLogicalAnd:
            if(lhs != booleanType)
                this->wrongType(lhs, booleanType,
                        &operation.getLeft()->getLocation()->getStart(),
                        &operation.getLeft()->getLocation()->getEnd());
            if(lhs != this->lastType)
                this->wrongType(this->lastType, lhs,
                        &operation.getRight()->getLocation()->getStart(),
                        &operation.getRight()->getLocation()->getEnd());
            this->lastType = booleanType;
            break;

        default:
            if(lhs != this->lastType)
                this->wrongType(lhs, this->lastType,
                        &operation.getRight()->getLocation()->getStart(),
                        &operation.getRight()->getLocation()->getEnd());
            this->lastType = booleanType;
            break;
    }
}

// basically like IProcedureCall but with return type
// checks actuals for expression type and returns function type if correct
void TypeChecker::visit(AST::EFunctionCall& call)
{
    // built-in function
    if(call.getName() == "writeln" || call.getName() == "write")
    {
        std::string name = call.getName();
        this->invalidCall(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());
    }

    if(call.getName() == "readln")
    {
        if(call.getActuals().size() != 0)
            this->invalidArity(call.getName(), &call.getLocation()->getStart(), &call.getLocation()->getEnd());

        this->lastType = integerType;
        return;
    }

    std::string name = call.getName();
    if(this->procedures.find(name) == this->procedures.end())
    {
        this->undefinedSymbol(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());
        return;
    }

    if(this->procedures[name].find(name) == this->procedures[name].end())
    {
        this->invalidCall(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());
        return;
    }

    // Is valid since we made sure the function exists above
    std::list<std::unique_ptr<AST::Procedure>>::const_iterator procedure =
        std::find_if(this->ast->getProcedures().begin(), this->ast->getProcedures().end(),
                     [&name](std::unique_ptr<AST::Procedure>& p){ return (p->getName() == name); });

    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>& formals = procedure->get()->getFormals();
    std::list<std::unique_ptr<AST::Expression>>& actuals = call.getActuals();

    if(formals.size() != actuals.size())
    {
        this->invalidArity(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());
        return;
    }

    auto formal = formals.begin();
    for(auto actual = actuals.begin() ;
            actual != actuals.end() ;
            formal++, actual++)
    {
        actual->get()->accept(*this);
        if(formal->second->getType() != this->lastType)
            this->wrongType(this->lastType, formal->second->getType(),
                    &actual->get()->getLocation()->getStart(), &actual->get()->getLocation()->getEnd());
    }

    if(procedure->get()->getResultType()->getType() == nullptr)
        this->invalidCall(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());

    this->lastType = procedure->get()->getResultType()->getType();
}

// checks index for int type and returns the value at accessed address' type
void TypeChecker::visit(AST::EArrayAccess& access)
{
    access.getIndex()->accept(*this);
    if(this->lastType != integerType)
        this->wrongType(this->lastType, integerType,
                &access.getIndex()->getLocation()->getStart(), &access.getIndex()->getLocation()->getEnd());

    access.getArray()->accept(*this);
    this->lastType = this->ast->getTypes()->get(this->lastType->kind, this->lastType->dimension - 1);
}

// checks size expression for integer type and returns allocated array type
void TypeChecker::visit(AST::EArrayAllocation& allocation)
{
    allocation.getElements()->accept(*this);
    if(this->lastType != integerType)
    {
        this->wrongType(this->lastType, integerType, &allocation.getElements()->getLocation()->getStart(),
                &allocation.getElements()->getLocation()->getEnd());
    }

    allocation.getType()->accept(*this);
}

void TypeChecker::visit(AST::Instruction&) { }

// basically like EFunctionCall but without return type
// checks arguments for correct type and makes sure callee has no return type
void TypeChecker::visit(AST::IProcedureCall& call)
{
    // Built-in procedure
    if(call.getName() == "writeln" || call.getName() == "write")
    {
        if(call.getActuals().size() != 1)
            this->invalidArity(call.getName(),
                    &call.getLocation()->getStart(),
                    &call.getLocation()->getEnd());

        call.getActuals().front()->accept(*this);

        if(this->lastType != integerType)
            this->wrongType(this->lastType, integerType,
                    &call.getActuals().front()->getLocation()->getStart(),
                    &call.getActuals().front()->getLocation()->getEnd());

        this->lastType = nullptr;
        return;
    }

    std::string name = call.getName();
    auto procedure = std::find_if(this->ast->getProcedures().begin(), this->ast->getProcedures().end(),
            [&name](std::unique_ptr<AST::Procedure>& currentProcedure){ return currentProcedure->getName() == name; });

    if(procedure == this->ast->getProcedures().end())
        this->undefinedSymbol(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());

    std::list<std::pair<std::string, std::unique_ptr<AST::PrimitiveType>>>& formals = procedure->get()->getFormals();
    std::list<std::unique_ptr<AST::Expression>>& actuals = call.getActuals();

    if(formals.size() != actuals.size())
        this->invalidArity(name, &actuals.front()->getLocation()->getStart(),
                &actuals.back()->getLocation()->getEnd());

    auto formal = formals.begin();
    for(auto actual = actuals.begin() ;
            actual != actuals.end() ;
            formal++, actual++)
    {
        actual->get()->accept(*this);
        if(formal->second->getType() != this->lastType)
            this->wrongType(this->lastType, formal->second->getType(),
                    &actual->get()->getLocation()->getStart(),
                    &actual->get()->getLocation()->getEnd());
    }

    if(procedure->get()->getResultType() != nullptr)
        this->invalidCall(name, &call.getLocation()->getStart(), &call.getLocation()->getEnd());

    this->lastType = nullptr;
}

// checks expression type and variable type and marks variable as initialized
void TypeChecker::visit(AST::IVariableAssignment& assignment)
{
    AST::PrimitiveType* type = nullptr;
    std::string name = assignment.getName();

    if(locals.find(name) != locals.end())
    {
        type = locals[name];
        this->localInitialized[name] = true;
    }
    else if(globals.find(name) != globals.end())
    {
        type = globals[name];
        this->globalInitialized[name] = true;
    }
    else
    {
        assignment.getValue()->accept(*this);
        this->undefinedSymbol(name, &assignment.getLocation()->getStart(),
                &assignment.getValue()->getLocation()->getStart());
    }
    assignment.getValue()->accept(*this);

    if(type == nullptr)
    {
        return;
    }
    else if(this->lastType != type->getType())
    {
        this->wrongType(this->lastType, type->getType(),
                &assignment.getValue()->getLocation()->getStart(),
                &assignment.getValue()->getLocation()->getEnd());
    }
}

// checks array dimension and index integer type and returns indexed expression type
void TypeChecker::visit(AST::IArrayAssignment& assignment)
{
    assignment.getArray()->accept(*this);
    const TOT::Type* arrayType = this->lastType;
    if(arrayType->dimension == 0)
    {
        this->invalidAssignment(arrayType,
                &assignment.getArray()->getLocation()->getStart(),
                &assignment.getArray()->getLocation()->getEnd());
    }

    assignment.getIndex()->accept(*this);
    const TOT::Type* indexType = this->lastType;
    if(indexType != integerType)
        this->wrongType(indexType, integerType,
                &assignment.getIndex()->getLocation()->getStart(),
                &assignment.getIndex()->getLocation()->getEnd());

    assignment.getValue()->accept(*this);
    const TOT::Type* valueType = this->lastType;
    const TOT::Type* indexedExpressionType = this->ast->getTypes()->get(arrayType->kind, arrayType->dimension - 1);
    if(indexedExpressionType != valueType)
        this->wrongType(valueType, indexedExpressionType,
                &assignment.getValue()->getLocation()->getStart(),
                &assignment.getValue()->getLocation()->getEnd());
}

// checks each instruction in sequence
void TypeChecker::visit(AST::ISequence& sequence)
{
    for(std::unique_ptr<AST::Instruction>& instruction : sequence.getInstructions())
        instruction->accept(*this);
}

// checks condition for boolean type and checks instructions
void TypeChecker::visit(AST::ICondition& condition)
{
    condition.getCondition()->accept(*this);
    if(this->lastType != booleanType)
        this->wrongType(this->lastType, booleanType,
                &condition.getCondition()->getLocation()->getStart(),
                &condition.getCondition()->getLocation()->getStart());

    condition.getTrue()->accept(*this);

    if(condition.getFalse().get() != nullptr)
        condition.getFalse()->accept(*this);
}

// checks condition for boolean type and checks instruction
void TypeChecker::visit(AST::IRepetition& repetition)
{
    repetition.getCondition()->accept(*this);
    if(this->lastType != booleanType)
        this->wrongType(this->lastType, booleanType, nullptr, nullptr);

    repetition.getInstructions()->accept(*this);
}

// defines formals, locals and result type/variable and checks body
void TypeChecker::visit(AST::Procedure& definition)
{
    this->localUsage.clear();
    this->localInitialized.clear();
    this->locals.clear();
    std::string name = definition.getName();
    this->currentFunction = name;


    if(definition.getResultType().get() != nullptr)
    {
        this->locals[name] = definition.getResultType().get();
        this->localUsage[name] = false;
        this->localInitialized[name] = false;
    }

    for(auto& argument : definition.getFormals())
    {
        if(this->locals.find(argument.first) == this->locals.end())
        {
            this->locals[argument.first] = argument.second.get();
            this->localInitialized[argument.first] = true;
        }
        else
            this->redefiningSymbol(argument.first, &argument.second->getLocation()->getStart(), nullptr);
    }

    for(auto& local : definition.getLocals())
    {
        if(this->locals.find(local.first) == this->locals.end())
        {
            this->locals[local.first] = local.second.get();
            this->localInitialized[local.first] = false;
            this->localUsage[local.first] = false;
        }
        else
            this->redefiningSymbol(local.first, &local.second->getLocation()->getStart(), nullptr);
    }

    definition.getBody()->accept(*this);

    for(auto& local : this->locals)
    {
        // Skip function name, for example functions that return a constant value
        if(local.first == definition.getName())
            continue;
        if(!this->localUsage[local.first])
            this->unusedValue(local.first, definition.getName(), nullptr, nullptr);
    }
}

// defines and checks globals and procedures and checks body
void TypeChecker::visit(AST::Program& program)
{
    this->globalInitialized.clear();
    this->globalUsage.clear();

    // Table of global variables
    for(auto& global : program.getGlobals())
    {
        if(this->globals.find(global.first) == this->globals.end())
        {
            this->globals[global.first] = global.second.get();
            this->globalUsage[global.first] = false;
            this->globalInitialized[global.first] = false;
        }
        else
            this->redefiningSymbol(global.first, &global.second->getLocation()->getStart(),
                    &global.second->getLocation()->getEnd());
    }

    // Checking declarations comes first since functions might call each other recursively
    for(const auto& procedure : program.getProcedures())
        this->readDeclaration(procedure.get());

    // Table of functions and procedures
    for(auto& procedure : program.getProcedures())
        procedure->accept(*this);

    this->locals.clear();
    this->currentFunction = "";
    program.getMain()->accept(*this);

    for(auto& global : this->globals)
    {
        if(!this->globalUsage[global.first])
            this->unusedValue(global.first, this->currentFunction, nullptr, nullptr);
    }
}

} // namespace pasclang::Semantic

