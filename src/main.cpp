#include "AST/AST.h"
#include "AST/PPPrinter.h"
#include "Parsing/Driver.h"
#include "Semantic/TypeChecker.h"
#include "Message/ConsoleReporter.h"
#include "LLVMBackend/IRGenerator.h"
#include "LLVMBackend/IROptimizer.h"
#include "LLVMBackend/ObjectGenerator.h"
#include <memory>
#include <iostream>

// temporary until safe linker call is implemented
#include <cstdlib>
//#include <lld/Driver/Driver.h>

#include "Pasclang.h"

int main(int argc, char* argv[])
{
    using namespace pasclang;
    ExitCode exitValue = ExitCode::Success;
    std::unique_ptr<Message::BaseReporter> reporter = std::make_unique<Message::ConsoleReporter>();
    std::string moduleName = "program";

    if(argc == 1)
    {
        std::cout << "Usage: " << argv[0] << " imputable.pp -o outputfile" << "\n"
            << "Options :\n\t-O0, -O1, -O2... - optimization level (only -O0 and -O1 actually do something for now)\n" <<
            "\t-S - emit LLVM IR assembly file to output instead of executable file\n" <<
            "\t-c - emit object file to output instead of executable file\n" <<
            "\t-p - source code formatting to standard output\n" <<
            "\t-d - dump LLVM IR assembly to standard error stream\n" <<
            "\t-f - only perform front-end tasks (lexical, syntactic and semantic analyses)" << std::endl;
        return ExitCode::WrongUsage;
    }

    bool verbose = false;
    std::string inputFile;
    std::string outputName;
    std::string outputFile;
    std::string outputExec = "a.out";
    unsigned int optimizationLevel = 0;
    bool link = true, assembly = false, print = false, dump = false, frontendOnly = false, needOutput = true;

    for(int argumentsIterator = 1; argumentsIterator < argc ; argumentsIterator++)
    {
        // It's a command
        if(argv[argumentsIterator][0] == '-')
        {
            switch (argv[argumentsIterator][1])
            {
                case 'o':
                    if(argc <= argumentsIterator + 1)
                    {
                        std::string errorMessage = "must provide a file name after -o";
                        reporter->message(Message::MessageType::Error, errorMessage, nullptr, nullptr);
                        return ExitCode::WrongUsage;
                    }
                    outputName = argv[argumentsIterator+1];
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
        else
        {
            inputFile = argv[argumentsIterator];
        }
    }

    if(inputFile == "")
    {
        std::string errorMessage = "no input file given, execute pasclang with no argument for usage";
        reporter->message(Message::MessageType::Error, errorMessage, nullptr, nullptr);
        return ExitCode::WrongUsage;
    }
    if(needOutput && outputName == "")
    {
        std::string errorMessage = "no output file given, execute pasclang with no argument for usage";
        reporter->message(Message::MessageType::Error, errorMessage, nullptr, nullptr);
        return ExitCode::WrongUsage;
    }

    if(!link)
        outputFile = outputName;
    else
    {
        outputExec = outputName;
        outputFile = outputName + ".o";
    }

    try {
        Parsing::Driver parsingDriver(reporter.get());
        std::unique_ptr<AST::Program> ast = parsingDriver.parse(inputFile);
        Semantic::TypeChecker tc(reporter.get());
        tc.check(ast);
        if(print)
        {
            AST::PPPrinter printer;
            printer.print(ast);
        }

        if(!frontendOnly)
        {
            LLVMBackend::IRGenerator ir(moduleName);
            ir.generate(ast);
            LLVMBackend::IROptimizer opt(optimizationLevel, ir.getModule(), reporter.get());
            LLVMBackend::ObjectGenerator obj(assembly, outputFile, ir.getModule(), reporter.get());
            if(dump)
                ir.getModule()->dump();
            if(link)
            {
#warning Placeholder until some fork/execlp method (and equivalent on non-Unix) is used instead
                std::string systemArg = PASCLANG_LINKER_DRIVER;
                systemArg += " -static " + outputFile + " -lpasclang-rt -L " + PASCLANG_RT_BUILD_PATH " -L " + PASCLANG_RT_INSTALL_PATH;
                systemArg += " -o " + outputExec;
                std::system(systemArg.c_str());
            }
        }
    }

    catch (PasclangException& e) {
        exitValue = e.getCode();
    }

    return exitValue;
}
