#include "LLVMBackend/ObjectGenerator.h"
#include "llvm/IR/LegacyPassManager.h"

#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/IR/Verifier.h>

namespace pasclang::LLVMBackend {

ObjectGenerator::ObjectGenerator(bool assembly, std::string& objectName, llvm::Module* module, Message::BaseReporter* reporter) :
    objectName(objectName), module(module), reporter(reporter)
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetDisassembler();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    this->objectName = objectName;
    this->triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    this->target = llvm::TargetRegistry::lookupTarget(this->triple, error);

    if(!this->target)
    {
        this->reporter->message(Message::MessageType::Error, error, nullptr, nullptr);
        throw PasclangException(ExitCode::GeneratorError);
    }

    std::string targetCpu = "generic";
    std::string cpuFeatures = "";

    llvm::TargetOptions targetOptions;
    llvm::Optional<llvm::Reloc::Model> relocModel = llvm::Optional<llvm::Reloc::Model>();
    llvm::TargetMachine* machine = this->target->createTargetMachine(this->triple, targetCpu, cpuFeatures, targetOptions, relocModel);

    this->module->setDataLayout(machine->createDataLayout());
    this->module->setTargetTriple(this->triple);

    std::string objectFile = this->objectName;
    std::error_code errorCode;
    llvm::raw_fd_ostream outputFile(objectFile, errorCode, llvm::sys::fs::F_None);
    if(errorCode)
    {
        std::string errorMessage = "Could not write to file " + objectFile;
        this->reporter->message(Message::MessageType::Error, errorMessage, nullptr, nullptr);
        throw PasclangException(ExitCode::GeneratorError);
    }

    llvm::legacy::PassManager passManager;
    llvm::TargetMachine::CodeGenFileType fileType = (assembly ? llvm::TargetMachine::CGFT_AssemblyFile : llvm::TargetMachine::CGFT_ObjectFile);

    if(machine->addPassesToEmitFile(passManager, outputFile, fileType))
    {
        std::string errorMessage = "Target machine cannot emit file of this type";
        this->reporter->message(Message::MessageType::Error, errorMessage, nullptr, nullptr);
        throw PasclangException(ExitCode::GeneratorError);
    }

    llvm::verifyModule(*this->module);
    passManager.run(*this->module);
    outputFile.flush();
}

} // namespace pasclang::LLVMBackend

