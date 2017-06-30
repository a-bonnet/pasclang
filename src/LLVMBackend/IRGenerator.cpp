#include "LLVMBackend/IRGenerator.h"

#include <llvm/IR/Verifier.h>

#include <string>
#include <vector>
#include <iostream>

namespace pasclang::LLVMBackend {

// converts PP type to LLVM type
llvm::Type* IRGenerator::astToLlvmType(AST::PrimitiveType* type)
{
    llvm::Type* resultType = nullptr;
    llvm::PointerType* pointerType = nullptr;

    switch(type->getType()->kind) {
        case AST::TableOfTypes::TypeKind::Boolean:

            resultType = llvm::Type::getInt1Ty(this->context);
            pointerType = static_cast<llvm::PointerType*>(resultType);
            for(std::uint32_t i = 0 ; i < type->getType()->dimension ; i++)
                pointerType = llvm::PointerType::get(pointerType, 0);
            break;

        case AST::TableOfTypes::TypeKind::Integer:

            resultType = llvm::Type::getInt32Ty(this->context);
            pointerType = static_cast<llvm::PointerType*>(resultType);
            for(std::uint32_t i = 0 ; i < type->getType()->dimension ; i++)
                pointerType = llvm::PointerType::get(pointerType, 0);
            break;

        default:
            throw PasclangException(ExitCode::GeneratorError);
    }

    return (pointerType == nullptr ? resultType : pointerType);
}

// emits LLVM IR for procedure declaration
llvm::Value* IRGenerator::emitDeclaration(AST::Procedure* definition)
{
    std::vector<llvm::Type*> argumentsTypes;
    argumentsTypes.reserve(definition->getFormals().size());
    for(auto& argument : definition->getFormals())
        argumentsTypes.push_back(this->astToLlvmType(argument.second.get()));

    llvm::FunctionType* procedureType;

    if(definition->getResultType().get() != nullptr)
        procedureType = llvm::FunctionType::get(this->astToLlvmType(definition->getResultType().get()),
            argumentsTypes, false);
    else
        procedureType = llvm::FunctionType::get(llvm::Type::getVoidTy(this->context),
            argumentsTypes, false);

    llvm::Function* procedure = llvm::Function::Create(procedureType, llvm::Function::ExternalLinkage,
            definition->getName(), this->module.get());

    auto formalsIterator = definition->getFormals().begin();
    for(auto argument = procedure->arg_begin() ; argument != procedure->arg_end() ; argument++)
    {
        argument->setName(formalsIterator->first);
        formalsIterator++;
    }

    return procedure;
}

// emits LLVM IR for global declaration
llvm::Value* IRGenerator::emitGlobal(std::string& name, llvm::Type* type)
{
    this->module->getOrInsertGlobal(name, type);
    this->globals[name] = this->module->getNamedGlobal(name);
    this->globals[name]->setLinkage(llvm::GlobalVariable::ExternalLinkage);

    if(type == llvm::Type::getInt1Ty(this->context))
        this->globals[name]->setInitializer(llvm::ConstantInt::getFalse(this->context));
    else if(type == llvm::Type::getInt32Ty(this->context))
        this->globals[name]->setInitializer(llvm::ConstantInt::get(type, 0, true));
    else this->globals[name]->setInitializer(llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(type)));

    return this->globals[name];
}

// emits main body
llvm::Function* IRGenerator::emitMain(std::unique_ptr<AST::Instruction>& main)
{
    std::vector<llvm::Type*> mainArgs; // empty for now
    llvm::FunctionType* mainType = llvm::FunctionType::get(llvm::Type::getVoidTy(this->context), mainArgs, false);
    llvm::Function* mainFunction = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", this->module.get());

    llvm::BasicBlock* mainEntry = llvm::BasicBlock::Create(this->context, "entry", mainFunction);
    this->builder.SetInsertPoint(mainEntry);

    main->accept(*this);

    this->builder.CreateRetVoid();

    return mainFunction;
}

// initializes LLVM module and builder
IRGenerator::IRGenerator(std::string& moduleName) : builder(llvm::IRBuilder<>(this->context))
{
    this->module = std::make_unique<llvm::Module>(moduleName, this->context);
}

void IRGenerator::generate(std::unique_ptr<AST::Program>& program)
{
    program->accept(*this);
}

// dumps LLVM IR assembly to stderr
void IRGenerator::dumpModule()
{
    this->module->dump();
}

void IRGenerator::visit(AST::PrimitiveType&) { }
void IRGenerator::visit(AST::Expression& expression) { }
void IRGenerator::visit(AST::EConstant& constant) { }

void IRGenerator::visit(AST::ECBoolean& boolean)
{
    this->lastValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(this->context), boolean.getValue() ? true : false, "bool");
}

void IRGenerator::visit(AST::ECInteger& integer)
{
    this->lastValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(this->context), integer.getValue(), "int");
}

// access from table of symbols
void IRGenerator::visit(AST::EVariableAccess& variable)
{
    llvm::Value* value;
    if(this->locals.find(variable.getName()) != this->locals.end())
        this->lastValue = this->builder.CreateLoad(this->locals[variable.getName()], "load");
    else
        this->lastValue = this->builder.CreateLoad(this->module->getNamedGlobal(variable.getName()), "load");
}

void IRGenerator::visit(AST::EUnaryOperation& operation)
{
    operation.getExpression()->accept(*this);

    switch(operation.getType())
    {
        case AST::EUnaryOperation::Type::UnaryMinus:
            this->lastValue = this->builder.CreateSub(llvm::ConstantInt::get(llvm::Type::getInt32Ty(this->context), 0),
                    this->lastValue, "minus");
            break;
        case AST::EUnaryOperation::Type::UnaryNot:
            this->lastValue = this->builder.CreateNot(this->lastValue, "not");
    }
}

void IRGenerator::visit(AST::EBinaryOperation& operation)
{
    llvm::Value *lhs, *rhs;

    if(operation.getType() != AST::EBinaryOperation::Type::BinaryLogicalOr &&
            operation.getType() != AST::EBinaryOperation::Type::BinaryLogicalAnd)
    {
        operation.getLeft()->accept(*this);
        lhs = this->lastValue;
        operation.getRight()->accept(*this);
        rhs = this->lastValue;

        switch(operation.getType())
        {
            case AST::EBinaryOperation::Type::BinaryAddition:
                this->lastValue = this->builder.CreateAdd(lhs, rhs, "add");
                break;
            case AST::EBinaryOperation::Type::BinarySubtraction:
                this->lastValue = this->builder.CreateSub(lhs, rhs, "sub");
                break;
            case AST::EBinaryOperation::Type::BinaryMultiplication:
                this->lastValue = this->builder.CreateMul(lhs, rhs, "mul");
                break;
            case AST::EBinaryOperation::Type::BinaryDivision:
                this->lastValue = this->builder.CreateSDiv(lhs, rhs, "div");
                break;
            case AST::EBinaryOperation::Type::BinaryLogicalLessThan:
                this->lastValue = this->builder.CreateICmpSLT(lhs, rhs, "lt");
                break;
            case AST::EBinaryOperation::Type::BinaryLogicalLessEqual:
                this->lastValue = this->builder.CreateICmpSLE(lhs, rhs, "le");
                break;
            case AST::EBinaryOperation::Type::BinaryLogicalGreaterThan:
                this->lastValue = this->builder.CreateICmpSGT(lhs, rhs, "gt");
                break;
            case AST::EBinaryOperation::Type::BinaryLogicalGreaterEqual:
                this->lastValue = this->builder.CreateICmpSGE(lhs, rhs, "ge");
                break;
            case AST::EBinaryOperation::Type::BinaryEquality:
                this->lastValue = this->builder.CreateICmpEQ(lhs, rhs, "eq");
                break;
            case AST::EBinaryOperation::Type::BinaryNonEquality:
                this->lastValue = this->builder.CreateICmpNE(lhs, rhs, "neq");
                break;
            default:
                break;
        }
    }
    // TODO: Short-circuiting logical operators
    else if(operation.getType() == AST::EBinaryOperation::Type::BinaryLogicalOr)
    {
        operation.getLeft()->accept(*this);
        lhs = this->lastValue;
        operation.getRight()->accept(*this);
        rhs = this->lastValue;

        this->lastValue = this->builder.CreateOr(lhs, rhs, "or_lhs");
    }
    else if(operation.getType() == AST::EBinaryOperation::Type::BinaryLogicalAnd)
    {
        operation.getLeft()->accept(*this);
        lhs = this->lastValue;
        operation.getRight()->accept(*this);
        rhs = this->lastValue;

        this->lastValue = this->builder.CreateAnd(lhs, rhs, "and_lhs");
    }
}

void IRGenerator::visit(AST::EFunctionCall& call)
{
    llvm::Function *callee = this->module->getFunction(call.getName());

    std::vector<llvm::Value*> arguments;
    for(auto& argument : call.getActuals())
    {
        argument->accept(*this);
        arguments.push_back(this->lastValue);
    }

    this->lastValue = this->builder.CreateCall(callee, arguments, "call");
}

void IRGenerator::visit(AST::EArrayAccess& access)
{
    access.getArray()->accept(*this);
    llvm::Value* array = this->lastValue;

    // we use a single-index GEP since every memory block is dynamically allocated
    std::vector<llvm::Value*> gepIndex(1);
    access.getIndex()->accept(*this);
    gepIndex[0] = this->lastValue;;

    this->lastValue = this->builder.CreateGEP(array, gepIndex, "gep");
    this->lastValue = this->builder.CreateLoad(this->lastValue, "loadptr");
}

// calls the runtime's allocation function
void IRGenerator::visit(AST::EArrayAllocation& allocation)
{
    // Allocation types are the following: (manually done since it's extern "C"'d)
    //  1 is for booleans
    //  2 is for integers
    //  3 is for pointers (aka multidimensional arrays)
    std::vector<llvm::Value*> arguments(2);
    allocation.getElements()->accept(*this);
    arguments[0] = this->lastValue;

    if(allocation.getType()->getType()->dimension > 1)
        arguments[1] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(this->context), 3);
    else if(allocation.getType()->getType()->kind == AST::TableOfTypes::TypeKind::Integer)
        arguments[1] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(this->context), 2);
    else
        arguments[1] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(this->context), 1);
    this->lastValue = this->builder.CreateCall(this->module->getFunction("pasclang_alloc"), arguments, "alloc");
}

void IRGenerator::visit(AST::Instruction& instruction) { }

void IRGenerator::visit(AST::IProcedureCall& call)
{
    llvm::Function *callee = this->module->getFunction(call.getName());
    std::vector<llvm::Value*> arguments;
    for(auto& argument : call.getActuals())
    {
        argument->accept(*this);
        arguments.push_back(this->lastValue);
    }
    this->lastValue = this->builder.CreateCall(callee, arguments);
}

// stores and makes sure value is properly casted for IR correctness
void IRGenerator::visit(AST::IVariableAssignment& assignment)
{
    llvm::Value* lhs;

    assignment.getValue()->accept(*this);
    llvm::Value* rhs = this->lastValue;

    if(this->locals.find(assignment.getName()) != this->locals.end())
        this->lastValue = this->locals[assignment.getName()];
    else
        this->lastValue = this->globals[assignment.getName()];

    lhs = this->lastValue;

    rhs = this->builder.CreateBitCast(rhs, static_cast<llvm::PointerType*>(lhs->getType())->getElementType(), "bitcast");
    this->lastValue = this->builder.CreateStore(rhs, lhs);
}

void IRGenerator::visit(AST::IArrayAssignment& assignment)
{
    assignment.getValue()->accept(*this);
    llvm::Value* value = this->lastValue;

    std::vector<llvm::Value*> gepIndex(1);
    assignment.getArray()->accept(*this);
    llvm::Value* array = this->lastValue;
    assignment.getIndex()->accept(*this);
    gepIndex[0] = this->lastValue;

    this->lastValue = this->builder.CreateGEP(array, gepIndex, "gep");
    value = this->builder.CreateBitCast(value, static_cast<llvm::PointerType*>(array->getType())->getElementType(), "arraybitcast");

    this->lastValue = this->builder.CreateStore(value, this->lastValue);
}

void IRGenerator::visit(AST::ISequence& sequence)
{
    for(auto& instruction : sequence.getInstructions())
        instruction->accept(*this);
}

void IRGenerator::visit(AST::ICondition& condition)
{
    condition.getCondition()->accept(*this);
    llvm::Value* conditionValue = this->lastValue;

    conditionValue = this->builder.CreateICmpNE(conditionValue, llvm::ConstantInt::getFalse(this->context), "test");

    llvm::Function* currentFunction = this->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* branchTrue = llvm::BasicBlock::Create(this->context, "true", currentFunction);
    llvm::BasicBlock* branchFalse = llvm::BasicBlock::Create(this->context, "false");
    llvm::BasicBlock* branchMerge = llvm::BasicBlock::Create(this->context, "merge");

    this->builder.CreateCondBr(conditionValue, branchTrue, branchFalse);
    this->builder.SetInsertPoint(branchTrue);

    condition.getTrue()->accept(*this);
    this->builder.CreateBr(branchMerge);
    branchTrue = this->builder.GetInsertBlock();

    currentFunction->getBasicBlockList().push_back(branchFalse);
    this->builder.SetInsertPoint(branchFalse);

    // note: if you don't check condition this will build but emit invalid IR and segfault
    if(condition.getFalse().get() != nullptr)
        condition.getFalse()->accept(*this);

    this->builder.CreateBr(branchMerge);
    branchFalse = this->builder.GetInsertBlock();

    currentFunction->getBasicBlockList().push_back(branchMerge);
    this->builder.SetInsertPoint(branchMerge);
}

void IRGenerator::visit(AST::IRepetition& repetition)
{
    repetition.getCondition()->accept(*this);
    llvm::Value* conditionValue = this->lastValue;
    conditionValue = this->builder.CreateICmpNE(conditionValue, llvm::ConstantInt::getFalse(this->context), "test");

    llvm::Function* currentFunction = this->builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* before = this->builder.GetInsertBlock();
    llvm::BasicBlock* loop = llvm::BasicBlock::Create(this->context, "loop", currentFunction);
    llvm::BasicBlock* end = llvm::BasicBlock::Create(this->context, "end", currentFunction);

    this->builder.CreateCondBr(conditionValue, loop, end);
    this->builder.SetInsertPoint(loop);

    repetition.getInstructions()->accept(*this);

    repetition.getCondition()->accept(*this);
    conditionValue = this->builder.CreateICmpNE(this->lastValue, llvm::ConstantInt::getFalse(this->context), "while");
    this->builder.CreateCondBr(conditionValue, loop, end);

    this->builder.SetInsertPoint(end);
}

void IRGenerator::visit(AST::Procedure& definition)
{
    this->locals.clear();
    llvm::Function* procedure = this->module->getFunction(definition.getName());

    auto formalsIterator = definition.getFormals().begin();
    for(auto argument = procedure->arg_begin() ; argument != procedure->arg_end() ; argument++)
    {
        argument->setName(formalsIterator->first);
        formalsIterator++;
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(this->context, "entry", procedure);
    this->builder.SetInsertPoint(entry);

    for(auto formal = procedure->arg_begin() ; formal != procedure->arg_end() ; formal++)
    {
        this->locals[formal->getName()] = this->builder.CreateAlloca(formal->getType(), nullptr, formal->getName());
        this->builder.CreateStore(&*formal, this->locals[formal->getName()], "storeformal");
    }

    if(definition.getResultType().get() != nullptr)
    {
        this->locals[definition.getName()] =
            this->builder.CreateAlloca(this->astToLlvmType(definition.getResultType().get()), nullptr, definition.getName());
    }

    for(auto& local : definition.getLocals())
    {
        this->locals[local.first] = this->builder.CreateAlloca(this->astToLlvmType(local.second.get()),
                nullptr, local.first);

        // Pseudo-Pascal semantics give default value to local variables too
        llvm::Value* defaultValue;
        if(local.second.get()->getType()->dimension > 0)
            defaultValue = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(this->astToLlvmType(local.second.get())));
        else if(local.second.get()->getType()->kind == AST::TableOfTypes::TypeKind::Integer)
            defaultValue = llvm::ConstantInt::getNullValue(this->astToLlvmType(local.second.get()));
        else
            defaultValue = llvm::ConstantInt::getFalse(this->context);
        this->builder.CreateStore(defaultValue, this->locals[local.first]);
    }

    definition.getBody()->accept(*this);

    if(definition.getResultType().get() == nullptr)
        this->builder.CreateRetVoid();
    else
    {
        llvm::Value* returnValue = this->builder.CreateLoad(this->locals[definition.getName()], "retval");
        this->builder.CreateRet(returnValue);
    }
}

void IRGenerator::visit(AST::Program& program)
{
    // Add built-in functions
    // write and writeln
    std::vector<llvm::Type*> writelnArgument(1);
    writelnArgument[0] = llvm::Type::getInt32Ty(this->context);
    llvm::FunctionType* writelnType = llvm::FunctionType::get(llvm::Type::getVoidTy(this->context), writelnArgument, false);
    llvm::Function::Create(writelnType, llvm::Function::ExternalLinkage, "writeln", this->module.get());
    llvm::Function::Create(writelnType, llvm::Function::ExternalLinkage, "write", this->module.get());
    // allocation from new
    std::vector<llvm::Type*> allocArguments(2);
    allocArguments[0] = llvm::Type::getInt32Ty(this->context);
    allocArguments[1] = llvm::Type::getInt8Ty(this->context);
    llvm::FunctionType* allocType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(this->context), allocArguments, false);
    llvm::Function::Create(allocType, llvm::Function::ExternalLinkage, "pasclang_alloc", this->module.get());
    // readln
    llvm::FunctionType* readlnType = llvm::FunctionType::get(llvm::Type::getInt32Ty(this->context), false);
    llvm::Function::Create(readlnType, llvm::Function::ExternalLinkage, "readln", this->module.get());

    for(auto& global : program.getGlobals())
        this->emitGlobal(global.first, this->astToLlvmType(global.second.get()));

    for(auto& procedure : program.getProcedures())
    {
        this->emitDeclaration(procedure.get());
    }

    for(auto& procedure : program.getProcedures())
        procedure->accept(*this);

    locals.clear();

    this->emitMain(program.getMain());

    llvm::verifyModule(*this->module.get());
}

} // namespace pasclang::LLVMBackend

