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

namespace {
std::array<llvm::PassBuilder::OptimizationLevel, 4> optimizationMap{
    {llvm::PassBuilder::O0, llvm::PassBuilder::O1, llvm::PassBuilder::O1,
     llvm::PassBuilder::O1}};
}

IROptimizer::IROptimizer(unsigned char optimizationLevel, llvm::Module* module,
                         Message::BaseReporter* reporter) {
    if (optimizationLevel == 0)
        return;

    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CAM;
    llvm::ModuleAnalysisManager MAM;

    llvm::PassBuilder passBuilder;
    passBuilder.registerModuleAnalyses(MAM);
    passBuilder.registerCGSCCAnalyses(CAM);
    passBuilder.registerFunctionAnalyses(FAM);
    passBuilder.registerLoopAnalyses(LAM);
    passBuilder.crossRegisterProxies(LAM, FAM, CAM, MAM);

    auto functionPassManager = passBuilder.buildFunctionSimplificationPipeline(
        optimizationMap[optimizationLevel],
        llvm::PassBuilder::ThinLTOPhase::None, false);

    if (optimizationLevel > 1) {
        std::string noteMessage =
            "optimization levels higher than 1 are currently equivalent to -O1";
        reporter->message(Message::MessageType::Note, noteMessage, nullptr,
                          nullptr);
    }

    for (auto& functionIterator : module->getFunctionList()) {
        if (!functionIterator.empty())
            functionPassManager.run(functionIterator, FAM);
    }
}

} // namespace pasclang::LLVMBackend
