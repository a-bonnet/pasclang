// This file contains the class making sure the program is well-typed. It does
// so by walking the tree and performing various checks by comparing the types
// obtained in each node as well as warning about potential flaws in the
// program.

#ifndef PASCLANG_SEMANTIC_TYPECHECKER_H
#define PASCLANG_SEMANTIC_TYPECHECKER_H

#include "AST/AST.h"
#include "Message/BaseReporter.h"
#include <vector>

namespace pasclang::Semantic {

// Works as a builder. Each procedure visits a node and updates the type
// checker's internal state.
// TODO: produce an untyped syntax tree for custom back-end generation
class TypeChecker : public AST::Visitor {
  public:
    typedef AST::TableOfTypes TOT;

  private:
    bool errorHappened = false;
    const TOT::Type* lastType;
    std::unique_ptr<AST::Program> ast;
    std::map<std::string, AST::PrimitiveType*> globals;
    std::map<std::string, std::map<std::string, AST::PrimitiveType*>>
        procedures;
    std::map<std::string, AST::PrimitiveType*> locals;
    std::map<std::string, bool> localUsage;
    std::map<std::string, bool> globalUsage;
    std::map<std::string, bool> localInitialized;
    std::map<std::string, bool> globalInitialized;
    std::string currentFunction;
    Message::BaseReporter* reporter;
    const TOT::Type* booleanType = nullptr;
    const TOT::Type* integerType = nullptr;

    // Errors and warnings
    void wrongType(const TOT::Type* type, const TOT::Type* expected,
                   const Parsing::Position* start,
                   const Parsing::Position* end);
    void wrongDimensions(size_t lhs, size_t rhs, const Parsing::Position* start,
                         const Parsing::Position* end);
    void invalidCall(std::string& name, const Parsing::Position* start,
                     const Parsing::Position* end);
    void invalidArity(std::string& name, const Parsing::Position* start,
                      const Parsing::Position* end);
    void undefinedSymbol(const std::string& symbol,
                         const Parsing::Position* start,
                         const Parsing::Position* end);
    void redefiningSymbol(const std::string& symbol,
                          const Parsing::Position* start,
                          const Parsing::Position* end);
    void invalidAssignment(const TOT::Type* type,
                           const Parsing::Position* start,
                           const Parsing::Position* end);
    void uninitializedValue(const std::string& symbol,
                            const std::string& function,
                            const Parsing::Position* start,
                            const Parsing::Position* end);
    void unusedValue(const std::string& symbol, const std::string& function,
                     const Parsing::Position* start,
                     const Parsing::Position* end);

    // Builds symbols declarations before definitions to allow several functions
    // recursively call each other.
    void readDeclaration(AST::Procedure* definition);

  public:
    TypeChecker(Message::BaseReporter* reporter) : reporter(reporter) {}
    virtual ~TypeChecker() {}

    // Not sure this needs to be public
    void swapAst(std::unique_ptr<AST::Program>& ast) {
        std::swap(ast, this->ast);
    }
    void check(std::unique_ptr<AST::Program>& ast);

    virtual void visit(AST::PrimitiveType& type) override;
    virtual void visit(AST::Expression& expression) override;
    virtual void visit(AST::EConstant& constant) override;
    virtual void visit(AST::ECBoolean& boolean) override;
    virtual void visit(AST::ECInteger& integer) override;
    virtual void visit(AST::EVariableAccess& variable) override;
    virtual void visit(AST::EUnaryOperation& operation) override;
    virtual void visit(AST::EBinaryOperation& operation) override;
    virtual void visit(AST::EFunctionCall& call) override;
    virtual void visit(AST::EArrayAccess& access) override;
    virtual void visit(AST::EArrayAllocation& allocation) override;
    virtual void visit(AST::Instruction& instruction) override;
    virtual void visit(AST::IProcedureCall& call) override;
    virtual void visit(AST::IVariableAssignment& assignment) override;
    virtual void visit(AST::IArrayAssignment& assignment) override;
    virtual void visit(AST::ISequence& sequence) override;
    virtual void visit(AST::ICondition& condition) override;
    virtual void visit(AST::IRepetition& repetition) override;
    virtual void visit(AST::Procedure& definition) override;
    virtual void visit(AST::Program& program) override;
};

} // namespace pasclang::Semantic

#endif
