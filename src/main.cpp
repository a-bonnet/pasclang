#include "AST/AST.h"
#include "AST/PPPrinter.h"
#include "LLVMBackend/IRGenerator.h"
#include "LLVMBackend/IROptimizer.h"
#include "LLVMBackend/ObjectGenerator.h"
#include "Message/ConsoleReporter.h"
#include "Parsing/Driver.h"
#include "Semantic/TypeChecker.h"
#include <iostream>
#include <memory>
#include <sstream>

#include <cstdlib>
//#include <lld/Driver/Driver.h>

#include "Pasclang.h"

int main(int argc, char* argv[]) {
    using namespace pasclang;
    ExitCode exitValue = ExitCode::Success;
    std::unique_ptr<Message::BaseReporter> reporter =
        std::make_unique<Message::ConsoleReporter>();
    std::string moduleName = "program";

    if (argc == 1) {
        std::cout
            << "Usage: " << argv[0] << " inputfile.pp -o outputfile"
            << "\n"
            << "Options :\n\t-O0, -O1, -O2... - optimization level (only -O0 "
               "and -O1 actually do something for now)\n"
            << "\t-S - emit LLVM IR assembly file to output instead of "
               "executable file\n"
            << "\t-c - emit object file to output instead of executable file\n"
            << "\t-p - source code formatting to standard output\n"
            << "\t-d - dump LLVM IR assembly to standard error stream\n"
            << "\t-f - only perform front-end tasks (lexical, syntactic and "
               "semantic analyses)"
            << std::endl;
        return ExitCode::WrongUsage;
    }

    bool verbose = false;
    std::string inputFile;
    std::string outputName;
    std::string outputFile;
    std::string outputExec = "a.out";
    unsigned int optimizationLevel = 0;
    bool link = true, assembly = false, print = false, dump = false,
         frontendOnly = false, needOutput = true;

    // Parsing flags
    for (int argumentsIterator = 1; argumentsIterator < argc;
         argumentsIterator++) {
        // It's a command
        if (argv[argumentsIterator][0] == '-') {
            switch (argv[argumentsIterator][1]) {
            case 'o':
                if (argc <= argumentsIterator + 1) {
                    std::string errorMessage =
                        "must provide a file name after -o";
                    reporter->message(Message::MessageType::Error, errorMessage,
                                      nullptr, nullptr);
                    return ExitCode::WrongUsage;
                }
                outputName = argv[argumentsIterator + 1];
                argumentsIterator++;
                break;
            case 'O':
                optimizationLevel = (argv[argumentsIterator][2] - '0');
                break;
            case 'c':
                link = false;
                break;
            case 'S':
                assembly = true;
                link = false;
                break;
            case 'p':
                print = true;
                frontendOnly = true;
                link = false;
                needOutput = false;
                break;
            case 'd':
                dump = true;
                break;
            case 'f':
                frontendOnly = true;
                needOutput = false;
            default:
                break;
            }
        }
        // It's the input
        else {
            inputFile = argv[argumentsIterator];
        }
    }

    if (inputFile == "") {
        std::string errorMessage =
            "no input file given, execute pasclang with no argument for usage";
        reporter->message(Message::MessageType::Error, errorMessage, nullptr,
                          nullptr);
        return ExitCode::WrongUsage;
    }

    if (needOutput && outputName == "") {
        std::string errorMessage =
            "no output file given, execute pasclang with no argument for usage";
        reporter->message(Message::MessageType::Error, errorMessage, nullptr,
                          nullptr);
        return ExitCode::WrongUsage;
    }
    if (outputName[0] == '-') {
        std::string errorMessage = "invalid output file format " + outputName;
        reporter->message(Message::MessageType::Error, errorMessage, nullptr,
                          nullptr);
        return ExitCode::WrongUsage;
    }

    // Checking if file exists and can be read
    std::ifstream fileCheck(inputFile.c_str());
    if (!fileCheck.good()) {
        std::string errorMessage = "could not open file " + inputFile;
        reporter->message(Message::MessageType::Error, errorMessage, nullptr,
                          nullptr);
        return ExitCode::InternalError;
    }
    fileCheck.close();

    // Main driver part
    if (!link)
        outputFile = outputName;
    else {
        outputExec = outputName;
        outputFile = outputName + ".o";
    }

    // Every component throws a pasclang::PasclangException after it is done if
    // something went wrong during its pass
    try {
        Parsing::Driver parsingDriver(reporter.get());
        std::unique_ptr<AST::Program> ast = parsingDriver.parse(inputFile);
        Semantic::TypeChecker tc(reporter.get());
        tc.check(ast);
        if (print) {
            AST::PPPrinter printer;
            printer.print(ast);
        }

        if (!frontendOnly) {
            LLVMBackend::IRGenerator ir(moduleName);
            ir.generate(*ast.get());
            LLVMBackend::IROptimizer opt(optimizationLevel, ir.getModule(),
                                         reporter.get());
            if (dump)
                ir.getModule()->print(llvm::errs(), nullptr);
            LLVMBackend::ObjectGenerator obj(assembly, outputFile,
                                             ir.getModule(), reporter.get());
            if (link) {
#warning Placeholder until some fork/execlp method (and equivalent on non-Unix) is used instead
                // The defines are provided by CMake
                std::ostringstream systemArg;
                systemArg << PASCLANG_LINKER_DRIVER << " -static " << outputFile
                          << " -lpasclang-rt -L " << PASCLANG_RT_BUILD_PATH
                          << " -L " << PASCLANG_RT_INSTALL_PATH << " -o "
                          << outputExec;
                std::system(systemArg.str().c_str());
            }
        }
    } catch (PasclangException& e) {
        exitValue = e.getCode();
    }

    return exitValue;
}
