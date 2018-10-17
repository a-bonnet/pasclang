#include "LLVMBackend/IROptimizer.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"
#include <iostream>

namespace pasclang::LLVMBackend {

IROptimizer::IROptimizer(unsigned char optimizationLevel, llvm::Module* module,
                         Message::BaseReporter* reporter)
    : optimizationLevel(optimizationLevel), module(module), reporter(reporter) {
    this->functionPassManager = std::make_unique<llvm::FunctionPassManager>();
    this->functionAnalysisManager =
        std::make_unique<llvm::FunctionAnalysisManager>();

    llvm::PassBuilder passBuilder;
    passBuilder.registerFunctionAnalyses(*functionAnalysisManager);

    if (this->optimizationLevel > 0) {
        // Example adding passes, notice PassBuilder<Function> has very
        // convenient methods to add the usual -O1, -O2... optimizations
        this->functionPassManager->addPass(llvm::SROA());
        this->functionPassManager->addPass(llvm::GVNHoistPass());
        this->functionPassManager->addPass(llvm::GVNSinkPass());
        this->functionPassManager->addPass(llvm::SimplifyCFGPass());
        this->functionPassManager->addPass(llvm::TailCallElimPass());
        this->functionPassManager->addPass(llvm::SimplifyCFGPass());
    }

    if (this->optimizationLevel > 1) {
        // Add more passes
        // In the meantime...
        std::string noteMessage =
            "optimization levels higher than 1 are currently equivalent to -O1";
        this->reporter->message(Message::MessageType::Note, noteMessage,
                                nullptr, nullptr);
    }

    for (auto functionIterator = this->module->getFunctionList().begin();
         functionIterator != this->module->getFunctionList().end();
         functionIterator++) {
        this->functionPassManager->run(*functionIterator,
                                       *functionAnalysisManager);
    }
}

} // namespace pasclang::LLVMBackend
