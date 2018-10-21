// Generates LLVM IR from the AST through the LLVM C++ API.

#ifndef PASCLANG_LLVMBACKEND_IRGENERATOR_H
#define PASCLANG_LLVMBACKEND_IRGENERATOR_H

#include "AST/AST.h"
#include "Pasclang.h"

#include <map>
#include <memory>
#include <string>

#include <llvm/ADT/APSInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

// print() method
#include <llvm/Support/raw_ostream.h>

#include <iostream>

namespace pasclang::LLVMBackend {

class IRGenerator final : public AST::Visitor {
  private:
    mutable llvm::Value* lastValue = nullptr;
    mutable llvm::LLVMContext context;
    mutable llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    mutable std::map<std::string, llvm::GlobalVariable*> globals;
    mutable std::map<std::string, llvm::AllocaInst*> locals;

    llvm::Value* emitDeclaration(const AST::Procedure* definition) const;
    llvm::Value* emitGlobal(const std::string& name, llvm::Type* type) const;
    llvm::Function* emitMain(const AST::Instruction& main) const;
    llvm::Type* astToLlvmType(const AST::PrimitiveType* type) const;

  public:
    IRGenerator(std::string& moduleName);
    ~IRGenerator() {}

    void generate(AST::Program& program);
    void dumpModule();
    llvm::Module* getModule() { return this->module.get(); }

    void visit(const AST::PrimitiveType& type) const override;
    void visit(const AST::Expression& expression) const override;
    void visit(const AST::EConstant& constant) const override;
    void visit(const AST::ECBoolean& boolean) const override;
    void visit(const AST::ECInteger& integer) const override;
    void visit(const AST::EVariableAccess& variable) const override;
    void visit(const AST::EUnaryOperation& operation) const override;
    void visit(const AST::EBinaryOperation& operation) const override;
    void visit(const AST::EFunctionCall& call) const override;
    void visit(const AST::EArrayAccess& access) const override;
    void visit(const AST::EArrayAllocation& allocation) const override;
    void visit(const AST::Instruction& instruction) const override;
    void visit(const AST::IProcedureCall& call) const override;
    void visit(const AST::IVariableAssignment& assignment) const override;
    void visit(const AST::IArrayAssignment& assignment) const override;
    void visit(const AST::ISequence& sequence) const override;
    void visit(const AST::ICondition& condition) const override;
    void visit(const AST::IRepetition& repetition) const override;
    void visit(const AST::Procedure& definition) const override;
    void visit(const AST::Program& program) const override;
};

} // namespace pasclang::LLVMBackend

#endif
