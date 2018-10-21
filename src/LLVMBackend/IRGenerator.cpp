#include "LLVMBackend/IRGenerator.h"

#include <llvm/IR/Verifier.h>

#include <iostream>
#include <string>
#include <vector>

namespace pasclang::LLVMBackend {

// converts PP type to LLVM type
llvm::Type* IRGenerator::astToLlvmType(const AST::PrimitiveType* type) const {
    llvm::Type* resultType = nullptr;
    llvm::PointerType* pointerType = nullptr;

    switch (type->getType()->kind) {
    case AST::TableOfTypes::TypeKind::Boolean:

        resultType = llvm::Type::getInt1Ty(this->context);
        pointerType = static_cast<llvm::PointerType*>(resultType);
        for (std::uint32_t i = 0; i < type->getType()->dimension; i++)
            pointerType = llvm::PointerType::get(pointerType, 0);
        break;

    case AST::TableOfTypes::TypeKind::Integer:

        resultType = llvm::Type::getInt32Ty(this->context);
        pointerType = static_cast<llvm::PointerType*>(resultType);
        for (std::uint32_t i = 0; i < type->getType()->dimension; i++)
            pointerType = llvm::PointerType::get(pointerType, 0);
        break;

    default:
        throw PasclangException(ExitCode::GeneratorError);
    }

    return (pointerType == nullptr ? resultType : pointerType);
}

// emits LLVM IR for procedure declaration so functions can be called in any
// order
llvm::Value*
IRGenerator::emitDeclaration(const AST::Procedure* definition) const {
    // LLVM functions are declared by giving a vector of argument types, a
    // result type and the procedure name
    std::vector<llvm::Type*> argumentsTypes;
    for (auto& argument : definition->getFormals())
        argumentsTypes.push_back(this->astToLlvmType(argument.second));

    llvm::FunctionType* procedureType;

    if (definition->getResultType() != nullptr)
        procedureType = llvm::FunctionType::get(
            this->astToLlvmType(definition->getResultType()), argumentsTypes,
            false);
    else
        procedureType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(this->context), argumentsTypes, false);

    // External linkage is not actually necessary, it can be changed to link
    // against other libraries with no trouble
    llvm::Function* procedure =
        llvm::Function::Create(procedureType, llvm::Function::ExternalLinkage,
                               definition->getName(), this->module.get());

    // Set the name for easier debugging/IR readability
    auto formals = definition->getFormals();
    auto f = formals.begin();
    for (auto argument = procedure->arg_begin();
         argument != procedure->arg_end(); argument++, f++) {
        argument->setName(f->first);
    }

    return procedure;
}

// emits LLVM IR for global declaration
llvm::Value* IRGenerator::emitGlobal(const std::string& name,
                                     llvm::Type* type) const {
    // This will insert since type checking makes sure we have no duplicate
    this->module->getOrInsertGlobal(name, type);
    this->globals[name] = this->module->getNamedGlobal(name);

    // Again, external linkage can be changed
    this->globals[name]->setLinkage(llvm::GlobalVariable::ExternalLinkage);

    // Globals should be initialized in PP, but also because IR will be invalid
    // otherwise
    if (type == llvm::Type::getInt1Ty(this->context))
        this->globals[name]->setInitializer(
            llvm::ConstantInt::getFalse(this->context));
    else if (type == llvm::Type::getInt32Ty(this->context))
        this->globals[name]->setInitializer(
            llvm::ConstantInt::get(type, 0, true));
    else
        this->globals[name]->setInitializer(llvm::ConstantPointerNull::get(
            static_cast<llvm::PointerType*>(type)));

    return this->globals[name];
}

// emits main body
llvm::Function* IRGenerator::emitMain(const AST::Instruction& main) const {
    std::vector<llvm::Type*> mainArgs; // empty for now, we could use command
                                       // line arguments for example
    // void return type means the program's main return value isn't a reliable
    // way to know whether the program terminated correctly
    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(this->context), mainArgs, false);
    llvm::Function* mainFunction = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", this->module.get());

    llvm::BasicBlock* mainEntry =
        llvm::BasicBlock::Create(this->context, "entry", mainFunction);
    this->builder.SetInsertPoint(mainEntry);

    // Generation with visitor
    main.accept(*this);

    this->builder.CreateRetVoid();

    return mainFunction;
}

// initializes LLVM module and builder
IRGenerator::IRGenerator(std::string& moduleName)
    : builder(llvm::IRBuilder<>(this->context)) {
    this->module = std::make_unique<llvm::Module>(moduleName, this->context);
}

void IRGenerator::generate(AST::Program& program) { program.accept(*this); }

// dumps LLVM IR assembly to stderr
void IRGenerator::dumpModule() { this->module->print(llvm::errs(), nullptr); }

void IRGenerator::visit(const AST::PrimitiveType&) const {}
void IRGenerator::visit(const AST::Expression& expression) const {}
void IRGenerator::visit(const AST::EConstant& constant) const {}
void IRGenerator::visit(const AST::ECBoolean& boolean) const {
    this->lastValue = llvm::ConstantInt::get(
        llvm::Type::getInt1Ty(const_cast<llvm::LLVMContext&>(this->context)),
        boolean.getValue(), "bool");
}

void IRGenerator::visit(const AST::ECInteger& integer) const {
    this->lastValue = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(this->context), integer.getValue(), "int");
}

// access from table of symbols
void IRGenerator::visit(const AST::EVariableAccess& variable) const {
    // Since everything is in memory by default and an optimization pass later
    // transforms instructions to SSA registers when possible, we always use the
    // load instruction
    if (this->locals.find(variable.getName()) != this->locals.end())
        this->lastValue =
            this->builder.CreateLoad(this->locals[variable.getName()], "load");
    else
        this->lastValue = this->builder.CreateLoad(
            this->module->getNamedGlobal(variable.getName()), "load");
}

void IRGenerator::visit(const AST::EUnaryOperation& operation) const {
    operation.getExpression().accept(*this);

    switch (operation.getType()) {
    // (-b) = (0-b)
    case AST::EUnaryOperation::Type::UnaryMinus:
        this->lastValue = this->builder.CreateSub(
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(this->context), 0),
            this->lastValue, "minus");
        break;
    case AST::EUnaryOperation::Type::UnaryNot:
        this->lastValue = this->builder.CreateNot(this->lastValue, "not");
    }
}

void IRGenerator::visit(const AST::EBinaryOperation& operation) const {
    llvm::Value *lhs, *rhs;

    if (operation.getType() != AST::EBinaryOperation::Type::BinaryLogicalOr &&
        operation.getType() != AST::EBinaryOperation::Type::BinaryLogicalAnd) {
        operation.getLeft().accept(*this);
        lhs = this->lastValue;
        operation.getRight().accept(*this);
        rhs = this->lastValue;

        switch (operation.getType()) {
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
    // Short-circuiting logical operators, the LLVM pseudo code is the
    // following: A or B: result in %c entry:
    //    %0 = alloca i1 result
    //    store i1 1, %0
    //    %a = i1 evaluate(A)
    //    br %a, end, next
    // next:       ; pred: entry
    //    %b = i1 evaluate(B)
    //    br %b, end, false
    // false:      ; pred: next
    //    store i1 0, %0
    //    br end
    // end:        ; pred: entry, next, false
    //    %c = load %0
    // and similar for and
    else if (operation.getType() ==
             AST::EBinaryOperation::Type::BinaryLogicalOr) {
        // the two lines after entry:
        llvm::AllocaInst* result = this->builder.CreateAlloca(
            llvm::Type::getInt1Ty(this->context), nullptr, "or_result");
        this->builder.CreateStore(llvm::ConstantInt::getTrue(this->context),
                                  result);

        // evaluate(A)
        operation.getLeft().accept(*this);
        lhs = this->lastValue;

        llvm::Function* currentFunction =
            this->builder.GetInsertBlock()->getParent();

        llvm::BasicBlock* next =
            llvm::BasicBlock::Create(this->context, "next", currentFunction);
        llvm::BasicBlock* final =
            llvm::BasicBlock::Create(this->context, "final");
        llvm::BasicBlock* end = llvm::BasicBlock::Create(this->context, "end");

        // br %a, end, next
        this->builder.CreateCondBr(lhs, end, next);
        this->builder.SetInsertPoint(next);

        // evaluate(B)
        operation.getRight().accept(*this);
        rhs = this->lastValue;
        // br %b, end, false
        this->builder.CreateCondBr(rhs, end, final);
        currentFunction->getBasicBlockList().push_back(final);
        this->builder.SetInsertPoint(final);

        // the false block
        this->builder.CreateStore(llvm::ConstantInt::getFalse(this->context),
                                  result);
        this->builder.CreateBr(end);
        currentFunction->getBasicBlockList().push_back(end);
        this->builder.SetInsertPoint(end);

        this->lastValue = this->builder.CreateLoad(result, "logicalor");
    }
    // See BinaryLogicalOr for similar description
    else if (operation.getType() ==
             AST::EBinaryOperation::Type::BinaryLogicalAnd) {
        llvm::AllocaInst* result = this->builder.CreateAlloca(
            llvm::Type::getInt1Ty(this->context), nullptr, "end_result");
        this->builder.CreateStore(llvm::ConstantInt::getFalse(this->context),
                                  result);

        operation.getLeft().accept(*this);
        lhs = this->lastValue;

        llvm::Function* currentFunction =
            this->builder.GetInsertBlock()->getParent();

        llvm::BasicBlock* next =
            llvm::BasicBlock::Create(this->context, "next", currentFunction);
        llvm::BasicBlock* final =
            llvm::BasicBlock::Create(this->context, "final");
        llvm::BasicBlock* end = llvm::BasicBlock::Create(this->context, "end");

        this->builder.CreateCondBr(lhs, next, end);
        this->builder.SetInsertPoint(next);

        operation.getRight().accept(*this);
        rhs = this->lastValue;
        this->builder.CreateCondBr(rhs, final, end);
        currentFunction->getBasicBlockList().push_back(final);
        this->builder.SetInsertPoint(final);

        this->builder.CreateStore(llvm::ConstantInt::getTrue(this->context),
                                  result);
        this->builder.CreateBr(end);
        currentFunction->getBasicBlockList().push_back(end);
        this->builder.SetInsertPoint(end);

        this->lastValue = this->builder.CreateLoad(result, "logicaland");
    }
}

void IRGenerator::visit(const AST::EFunctionCall& call) const {
    llvm::Function* callee = this->module->getFunction(call.getName());

    // Calls function by evaluating arguments (the so-called actuals) from left
    // to right
    std::vector<llvm::Value*> arguments;
    for (auto& argument : call.getActuals()) {
        argument->accept(*this);
        arguments.push_back(this->lastValue);
    }

    this->lastValue = this->builder.CreateCall(callee, arguments, "call");
}

void IRGenerator::visit(const AST::EArrayAccess& access) const {
    access.getArray().accept(*this);
    // We first need to know the array's address
    llvm::Value* array = this->lastValue;

    // we use a single-index GEP since every memory block is dynamically
    // allocated, this is the offset from the array's start for the array's type
    std::vector<llvm::Value*> gepIndex(1);
    access.getIndex().accept(*this);
    gepIndex[0] = this->lastValue;

    // GEP only computes the address, we still need to load the value from the
    // address
    this->lastValue = this->builder.CreateGEP(array, gepIndex, "gep");
    this->lastValue = this->builder.CreateLoad(this->lastValue, "loadptr");
}

// calls the runtime's allocation function
void IRGenerator::visit(const AST::EArrayAllocation& allocation) const {
    // Allocation types are the following: (manually done since it's extern
    // "C"'d)
    //  1 is for booleans
    //  2 is for integers
    //  3 is for pointers (aka multidimensional arrays)
    std::vector<llvm::Value*> arguments(2);
    allocation.getElements().accept(*this);
    arguments[0] = this->lastValue;

    if (allocation.getType().getType()->dimension > 1)
        arguments[1] =
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(this->context), 3);
    else if (allocation.getType().getType()->kind ==
             AST::TableOfTypes::TypeKind::Integer)
        arguments[1] =
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(this->context), 2);
    else
        arguments[1] =
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(this->context), 1);
    this->lastValue = this->builder.CreateCall(
        this->module->getFunction("__pasclang_gc_alloc"), arguments, "alloc");
}

void IRGenerator::visit(const AST::Instruction& instruction) const {}

void IRGenerator::visit(const AST::IProcedureCall& call) const {
    llvm::Function* callee = this->module->getFunction(call.getName());
    std::vector<llvm::Value*> arguments;
    // Like in functions, we evaluate arguments from left to right
    for (auto& argument : call.getActuals()) {
        argument->accept(*this);
        arguments.push_back(this->lastValue);
    }
    // Note procedures don't actually return a value so affectation would be
    // dead code
    this->builder.CreateCall(callee, arguments);
}

// stores and makes sure value is properly cast for IR correctness
void IRGenerator::visit(const AST::IVariableAssignment& assignment) const {
    llvm::Value* lhs;

    assignment.getValue().accept(*this);
    llvm::Value* rhs = this->lastValue;

    if (this->locals.find(assignment.getName()) != this->locals.end())
        this->lastValue = this->locals[assignment.getName()];
    else
        this->lastValue = this->globals[assignment.getName()];

    lhs = this->lastValue;

    // This gives a type to the bytes loaded from address
    rhs = this->builder.CreateBitCast(
        rhs, static_cast<llvm::PointerType*>(lhs->getType())->getElementType(),
        "bitcast");
    this->lastValue = this->builder.CreateStore(rhs, lhs);
}

void IRGenerator::visit(const AST::IArrayAssignment& assignment) const {
    // This is similar to array access to know the address
    assignment.getValue().accept(*this);
    llvm::Value* value = this->lastValue;

    std::vector<llvm::Value*> gepIndex(1);

    auto& arrayAccess =
        static_cast<const AST::EArrayAccess&>(assignment.getArray());

    arrayAccess.getArray().accept(*this);
    llvm::Value* array = this->lastValue;

    arrayAccess.getIndex().accept(*this);
    gepIndex[0] = this->lastValue;

    this->lastValue = this->builder.CreateGEP(array, gepIndex, "gep");
    value = this->builder.CreateBitCast(
        value,
        static_cast<llvm::PointerType*>(array->getType())->getElementType(),
        "arraybitcast");

    // we just happen to add the storing of rhs
    this->lastValue = this->builder.CreateStore(value, this->lastValue);
}

void IRGenerator::visit(const AST::ISequence& sequence) const {
    // Each instruction is executed from first to last
    for (auto& instruction : sequence.getInstructions())
        instruction->accept(*this);
}

void IRGenerator::visit(const AST::ICondition& condition) const {
    // First we need to evaluate the condition to know which branch is taken
    condition.getCondition().accept(*this);
    llvm::Value* conditionValue = this->lastValue;

    // Conditions are supposed to be booleans, we compare the result to false
    conditionValue = this->builder.CreateICmpNE(
        conditionValue, llvm::ConstantInt::getFalse(this->context), "test");

    llvm::Function* currentFunction =
        this->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* branchTrue =
        llvm::BasicBlock::Create(this->context, "true", currentFunction);
    llvm::BasicBlock* branchFalse =
        llvm::BasicBlock::Create(this->context, "false");
    llvm::BasicBlock* branchMerge =
        llvm::BasicBlock::Create(this->context, "merge");

    // Jump to the correct branch
    this->builder.CreateCondBr(conditionValue, branchTrue, branchFalse);
    this->builder.SetInsertPoint(branchTrue);

    condition.getTrue().accept(*this);
    this->builder.CreateBr(branchMerge);

    // Don't forget the false basic block hasn't yet been attached to a
    // function, we do this now
    currentFunction->getBasicBlockList().push_back(branchFalse);
    this->builder.SetInsertPoint(branchFalse);

    // note: if you don't check condition this will build but emit invalid IR
    // and segfault, worst case here is it gives a dead code branch which easily
    // gets eaten by optimizers
    if (condition.getFalse() != nullptr)
        condition.getFalse()->accept(*this);

    this->builder.CreateBr(branchMerge);
    branchFalse = this->builder.GetInsertBlock();

    // Like with the false basic block, we need to attach the merge block
    currentFunction->getBasicBlockList().push_back(branchMerge);
    this->builder.SetInsertPoint(branchMerge);
}

void IRGenerator::visit(const AST::IRepetition& repetition) const {
    // First we evaluate the condition
    repetition.getCondition().accept(*this);
    llvm::Value* conditionValue = this->lastValue;
    conditionValue = this->builder.CreateICmpNE(
        conditionValue, llvm::ConstantInt::getFalse(this->context), "test");

    llvm::Function* currentFunction =
        this->builder.GetInsertBlock()->getParent();
    // create two blocks, one for the loop's content and the other one to jump
    // out of the loop when condition gets false
    llvm::BasicBlock* loop =
        llvm::BasicBlock::Create(this->context, "loop", currentFunction);
    llvm::BasicBlock* end =
        llvm::BasicBlock::Create(this->context, "end", currentFunction);

    this->builder.CreateCondBr(conditionValue, loop, end);
    this->builder.SetInsertPoint(loop);

    repetition.getInstructions().accept(*this);

    // Need to check condition again at the end of block to know whether we're
    // looping or breaking
    repetition.getCondition().accept(*this);
    conditionValue = this->builder.CreateICmpNE(
        this->lastValue, llvm::ConstantInt::getFalse(this->context), "while");
    this->builder.CreateCondBr(conditionValue, loop, end);

    // bye
    this->builder.SetInsertPoint(end);
}

void IRGenerator::visit(const AST::Procedure& definition) const {
    // The procedures are already declared, this is necessary so we can call
    // procedures declared later a consequence is we don't need to specify the
    // prototype again
    this->locals.clear();
    llvm::Function* procedure = this->module->getFunction(definition.getName());

    llvm::BasicBlock* entry =
        llvm::BasicBlock::Create(this->context, "entry", procedure);
    this->builder.SetInsertPoint(entry);

    // We set formals in memory again, this might look expensive but SSA
    // optimizations will change it if it's not needed.
    for (auto formal = procedure->arg_begin(); formal != procedure->arg_end();
         formal++) {
        this->locals[formal->getName()] = this->builder.CreateAlloca(
            formal->getType(), nullptr, formal->getName());
        this->builder.CreateStore(&*formal, this->locals[formal->getName()],
                                  "storeformal");
    }

    // The function's return value is the variable bound to its name
    if (definition.getResultType() != nullptr) {
        this->locals[definition.getName()] = this->builder.CreateAlloca(
            this->astToLlvmType(definition.getResultType()), nullptr,
            definition.getName());
    }

    // Local variables are stored on the stack too
    for (auto& local : definition.getLocals()) {
        this->locals[local.first] = this->builder.CreateAlloca(
            this->astToLlvmType(local.second), nullptr, local.first);

        // Pseudo-Pascal semantics give default value to local variables, we
        // proceed like we did for globals
        llvm::Value* defaultValue;
        if (local.second->getType()->dimension > 0)
            defaultValue =
                llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(
                    this->astToLlvmType(local.second)));
        else if (local.second->getType()->kind ==
                 AST::TableOfTypes::TypeKind::Integer)
            defaultValue = llvm::ConstantInt::getNullValue(
                this->astToLlvmType(local.second));
        else
            defaultValue = llvm::ConstantInt::getFalse(this->context);
        this->builder.CreateStore(defaultValue, this->locals[local.first]);
    }

    definition.getBody().accept(*this);

    if (definition.getResultType() == nullptr)
        this->builder.CreateRetVoid();
    else {
        llvm::Value* returnValue = this->builder.CreateLoad(
            this->locals[definition.getName()], "retval");
        this->builder.CreateRet(returnValue);
    }
}

void IRGenerator::visit(const AST::Program& program) const {
    // Add built-in functions
    // write and writeln
    std::vector<llvm::Type*> writelnArgument(1);
    writelnArgument[0] = llvm::Type::getInt32Ty(this->context);
    llvm::FunctionType* writelnType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(this->context), writelnArgument, false);
    llvm::Function::Create(writelnType, llvm::Function::ExternalLinkage,
                           "writeln", this->module.get());
    llvm::Function::Create(writelnType, llvm::Function::ExternalLinkage,
                           "write", this->module.get());
    // allocation from new
    std::vector<llvm::Type*> allocArguments(2);
    allocArguments[0] = llvm::Type::getInt32Ty(this->context);
    allocArguments[1] = llvm::Type::getInt8Ty(this->context);
    llvm::FunctionType* allocType = llvm::FunctionType::get(
        llvm::Type::getInt8PtrTy(this->context), allocArguments, false);
    llvm::Function::Create(allocType, llvm::Function::ExternalLinkage,
                           "__pasclang_gc_alloc", this->module.get());
    // readln
    llvm::FunctionType* readlnType =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(this->context), false);
    llvm::Function::Create(readlnType, llvm::Function::ExternalLinkage,
                           "readln", this->module.get());

    for (auto& global : program.getGlobals())
        this->emitGlobal(global.first, this->astToLlvmType(global.second));

    for (auto* procedure : program.getProcedures()) {
        this->emitDeclaration(procedure);
    }

    for (auto& procedure : program.getProcedures())
        procedure->accept(*this);

    locals.clear();

    this->emitMain(program.getMain());

    llvm::verifyModule(*this->module.get());
}

} // namespace pasclang::LLVMBackend
