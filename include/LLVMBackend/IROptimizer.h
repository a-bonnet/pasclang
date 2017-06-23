#ifndef PASCLANG_LLVMBACKEND_IROPTIMIZER_H
#define PASCLANG_LLVMBACKEND_IROPTIMIZER_H

#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <memory>

#include "Message/BaseReporter.h"
#include "Pasclang.h"

namespace pasclang::LLVMBackend {

class IROptimizer
{
    private:
        unsigned char optimizationLevel;
        std::unique_ptr<llvm::legacy::FunctionPassManager> functionPassManager;
        llvm::Module* module;
        Message::BaseReporter* reporter;

    public:
        IROptimizer(unsigned char optimizationLevel, llvm::Module* module, Message::BaseReporter* reporter);
        ~IROptimizer() {}
};

}

#endif

