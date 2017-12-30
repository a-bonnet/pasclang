// Outputs LLVM module as either an object file for linking
// or LLVM IR assembly for debugging/analysis.

#ifndef PASCLANG_LLVMBACKEND_OBJECTGENERATOR_H
#define PASCLANG_LLVMBACKEND_OBJECTGENERATOR_H

#include "LLVMBackend/IRGenerator.h"
#include <llvm/Target/TargetMachine.h>

#include "Message/BaseReporter.h"
#include "Pasclang.h"
#include <iostream>

namespace pasclang::LLVMBackend {

class ObjectGenerator {
  private:
    std::string& objectName;
    std::string triple;
    const llvm::Target* target;
    llvm::Module* module;
    Message::BaseReporter* reporter;

  public:
    ObjectGenerator(bool assembly, std::string& objectName,
                    llvm::Module* module, Message::BaseReporter* reporter);
    ~ObjectGenerator() {}
};

} // namespace pasclang::LLVMBackend

#endif
