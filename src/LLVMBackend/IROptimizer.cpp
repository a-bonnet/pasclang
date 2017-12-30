#include "LLVMBackend/IROptimizer.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include <iostream>

namespace pasclang::LLVMBackend {

IROptimizer::IROptimizer(unsigned char optimizationLevel, llvm::Module* module,
                         Message::BaseReporter* reporter)
    : optimizationLevel(optimizationLevel), module(module), reporter(reporter) {
    this->functionPassManager =
        std::make_unique<llvm::legacy::FunctionPassManager>(module);

    if (this->optimizationLevel > 0) {
        this->functionPassManager->add(
            llvm::createPromoteMemoryToRegisterPass());
        this->functionPassManager->add(llvm::createDeadCodeEliminationPass());
        this->functionPassManager->add(llvm::createGVNPass());
        this->functionPassManager->add(llvm::createInstructionCombiningPass());
        this->functionPassManager->add(llvm::createCFGSimplificationPass());
        this->functionPassManager->add(llvm::createReassociatePass());
    }

    if (this->optimizationLevel > 1) {
        // Add more passes
        // In the meantime...
        std::string noteMessage =
            "optimization levels higher than 1 are currently equivalent to -O1";
        this->reporter->message(Message::MessageType::Note, noteMessage,
                                nullptr, nullptr);
    }

    this->functionPassManager->doInitialization();

    for (auto functionIterator = this->module->getFunctionList().begin();
         functionIterator != this->module->getFunctionList().end();
         functionIterator++) {
        this->functionPassManager->run(*functionIterator);
    }
}

} // namespace pasclang::LLVMBackend
