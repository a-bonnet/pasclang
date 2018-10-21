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
    mutable bool errorHappened = false;
    mutable const TOT::Type* lastType;
    mutable std::unique_ptr<AST::Program> ast;
    mutable std::map<std::string, const AST::PrimitiveType*> globals;
    mutable std::map<std::string,
                     std::map<std::string, const AST::PrimitiveType*>>
        procedures;
    mutable std::map<std::string, const AST::PrimitiveType*> locals;
    mutable std::map<std::string, bool> localUsage;
    mutable std::map<std::string, bool> globalUsage;
    mutable std::map<std::string, bool> localInitialized;
    mutable std::map<std::string, bool> globalInitialized;
    mutable std::string currentFunction;
    Message::BaseReporter* reporter;
    mutable const TOT::Type* booleanType = nullptr;
    mutable const TOT::Type* integerType = nullptr;

    // Errors and warnings
    void wrongType(const TOT::Type* type, const TOT::Type* expected,
                   const Parsing::Position* start,
                   const Parsing::Position* end) const;
    void wrongDimensions(size_t lhs, size_t rhs, const Parsing::Position* start,
                         const Parsing::Position* end) const;
    void invalidCall(std::string& name, const Parsing::Position* start,
                     const Parsing::Position* end) const;
    void invalidArity(const std::string& name, const Parsing::Position* start,
                      const Parsing::Position* end) const;
    void undefinedSymbol(const std::string& symbol,
                         const Parsing::Position* start,
                         const Parsing::Position* end) const;
    void redefiningSymbol(const std::string& symbol,
                          const Parsing::Position* start,
                          const Parsing::Position* end) const;
    void invalidAssignment(const TOT::Type* type,
                           const Parsing::Position* start,
                           const Parsing::Position* end) const;
    void uninitializedValue(const std::string& symbol,
                            const std::string& function,
                            const Parsing::Position* start,
                            const Parsing::Position* end) const;
    void unusedValue(const std::string& symbol, const std::string& function,
                     const Parsing::Position* start,
                     const Parsing::Position* end) const;

    // Builds symbols declarations before definitions to allow several functions
    // recursively call each other.
    void readDeclaration(const AST::Procedure* definition) const;

  public:
    TypeChecker(Message::BaseReporter* reporter) : reporter(reporter) {}
    virtual ~TypeChecker() {}

    // Not sure this needs to be public
    void swapAst(std::unique_ptr<AST::Program>& ast) {
        std::swap(ast, this->ast);
    }
    void check(std::unique_ptr<AST::Program>& ast);

    virtual void visit(const AST::PrimitiveType& type) const override;
    virtual void visit(const AST::Expression& expression) const override;
    virtual void visit(const AST::EConstant& constant) const override;
    virtual void visit(const AST::ECBoolean& boolean) const override;
    virtual void visit(const AST::ECInteger& integer) const override;
    virtual void visit(const AST::EVariableAccess& variable) const override;
    virtual void visit(const AST::EUnaryOperation& operation) const override;
    virtual void visit(const AST::EBinaryOperation& operation) const override;
    virtual void visit(const AST::EFunctionCall& call) const override;
    virtual void visit(const AST::EArrayAccess& access) const override;
    virtual void visit(const AST::EArrayAllocation& allocation) const override;
    virtual void visit(const AST::Instruction& instruction) const override;
    virtual void visit(const AST::IProcedureCall& call) const override;
    virtual void
    visit(const AST::IVariableAssignment& assignment) const override;
    virtual void visit(const AST::IArrayAssignment& assignment) const override;
    virtual void visit(const AST::ISequence& sequence) const override;
    virtual void visit(const AST::ICondition& condition) const override;
    virtual void visit(const AST::IRepetition& repetition) const override;
    virtual void visit(const AST::Procedure& definition) const override;
    virtual void visit(const AST::Program& program) const override;
};

} // namespace pasclang::Semantic

#endif
