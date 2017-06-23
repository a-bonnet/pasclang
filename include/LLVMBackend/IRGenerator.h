#ifndef PASCLANG_LLVMBACKEND_IRGENERATOR_H
#define PASCLANG_LLVMBACKEND_IRGENERATOR_H

#include "AST/AST.h"
#include "Pasclang.h"

#include <memory>
#include <map>
#include <string>

#include <llvm/ADT/APSInt.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/GlobalVariable.h>

#include <iostream>

namespace pasclang::LLVMBackend {

class IRGenerator final : public AST::Visitor
{
    private:
        llvm::Value* lastValue = nullptr;
        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        std::map<std::string, llvm::GlobalVariable*> globals;
        std::map<std::string, llvm::AllocaInst*> locals;

        llvm::Value* emitGlobal(std::string& name, llvm::Type* type);
        llvm::Function* emitMain(std::unique_ptr<AST::Instruction>& main);
        llvm::Type* astToLlvmType(AST::PrimitiveType* type);

    public:
        IRGenerator(std::string& moduleName);
        ~IRGenerator() { }

        void generate(std::unique_ptr<AST::Program>& program);
        void dumpModule();
        llvm::Module* getModule() { return this->module.get(); }

        void visit(AST::PrimitiveType& type) override;
        void visit(AST::Expression& expression) override;
        void visit(AST::EConstant& constant) override;
        void visit(AST::ECBoolean& boolean) override;
        void visit(AST::ECInteger& integer) override;
        void visit(AST::EVariableAccess& variable) override;
        void visit(AST::EUnaryOperation& operation) override;
        void visit(AST::EBinaryOperation& operation) override;
        void visit(AST::EFunctionCall& call) override;
        void visit(AST::EArrayAccess& access) override;
        void visit(AST::EArrayAllocation& allocation) override;
        void visit(AST::Instruction& instruction) override;
        void visit(AST::IProcedureCall& call) override;
        void visit(AST::IVariableAssignment& assignment) override;
        void visit(AST::IArrayAssignment& assignment) override;
        void visit(AST::ISequence& sequence) override;
        void visit(AST::ICondition& condition) override;
        void visit(AST::IRepetition& repetition) override;
        void visit(AST::Procedure& definition) override;
        void visit(AST::Program& program) override;
};

} // namespace pasclang::LLVMBackend

#endif

