#include "LLVMBackend/ObjectGenerator.h"

#include <llvm/CodeGen/BuiltinGCs.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>

namespace pasclang::LLVMBackend {

ObjectGenerator::ObjectGenerator(bool assembly, std::string& objectName,
                                 llvm::Module* module,
                                 Message::BaseReporter* reporter)
    : objectName(objectName), module(module), reporter(reporter) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    // Triple (e.g. x86_64-unknown-linux-gnu) information is required to set the
    // target
    this->objectName = objectName;
    this->triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    this->target = llvm::TargetRegistry::lookupTarget(this->triple, error);

    if (!this->target) {
        this->reporter->message(Message::MessageType::Error, error, nullptr,
                                nullptr);
        throw PasclangException(ExitCode::GeneratorError);
    }

    std::string targetCpu = "generic";
    std::string cpuFeatures = "";

    llvm::TargetOptions targetOptions;
    llvm::TargetMachine* machine = this->target->createTargetMachine(
        this->triple, targetCpu, cpuFeatures, targetOptions,
        llvm::Optional<llvm::Reloc::Model>());

    this->module->setDataLayout(machine->createDataLayout());
    this->module->setTargetTriple(this->triple);

    std::string objectFile = this->objectName;
    std::error_code errorCode;
    llvm::raw_fd_ostream outputFile(objectFile, errorCode,
                                    llvm::sys::fs::F_None);
    if (errorCode) {
        std::string errorMessage = "Could not write to file " + objectFile;
        this->reporter->message(Message::MessageType::Error, errorMessage,
                                nullptr, nullptr);
        throw PasclangException(ExitCode::GeneratorError);
    }

    llvm::legacy::PassManager passManager;
    llvm::CodeGenFileType fileType =
        (assembly ? llvm::CodeGenFileType::CGFT_AssemblyFile
                  : llvm::CodeGenFileType::CGFT_ObjectFile);

    if (machine->addPassesToEmitFile(passManager, outputFile, nullptr,
                                     fileType)) {
        std::string errorMessage =
            "Target machine cannot emit file of this type";
        this->reporter->message(Message::MessageType::Error, errorMessage,
                                nullptr, nullptr);
        throw PasclangException(ExitCode::GeneratorError);
    }

    //llvm::linkAllBuiltinGCs();

    llvm::verifyModule(*this->module);
    passManager.run(*this->module);
    outputFile.flush();
}

} // namespace pasclang::LLVMBackend
