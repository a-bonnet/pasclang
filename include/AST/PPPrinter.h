// Implements a source code printer from AST, note it removes comments since
// they are not parsed.

#ifndef PASCLANG_AST_PRINTER_H
#define PASCLANG_AST_PRINTER_H

#include "AST/AST.h"
#include <iostream>
#include <sstream>

namespace pasclang::AST {

class PPPrinter : public Visitor {
  private:
    mutable size_t indentation = 0;
    mutable std::ostringstream buffer;

  public:
    PPPrinter() {}
    virtual ~PPPrinter() {}
    void print(std::unique_ptr<AST::Program>& program) const;
    void indent() const;

    virtual void visit(const PrimitiveType& type) const;
    virtual void visit(const Expression& expression) const;
    virtual void visit(const EConstant& constant) const;
    virtual void visit(const ECBoolean& boolean) const;
    virtual void visit(const ECInteger& integer) const;
    virtual void visit(const EVariableAccess& variable) const;
    virtual void visit(const EUnaryOperation& operation) const;
    virtual void visit(const EBinaryOperation& operation) const;
    virtual void visit(const EFunctionCall& call) const;
    virtual void visit(const EArrayAccess& access) const;
    virtual void visit(const EArrayAllocation& allocation) const;
    virtual void visit(const Instruction& instruction) const;
    virtual void visit(const IProcedureCall& call) const;
    virtual void visit(const IVariableAssignment& assignment) const;
    virtual void visit(const IArrayAssignment& assignment) const;
    virtual void visit(const ISequence& sequence) const;
    virtual void visit(const ICondition& condition) const;
    virtual void visit(const IRepetition& repetition) const;
    virtual void visit(const Procedure& definition) const;
    virtual void visit(const Program& program) const;
};

} // namespace pasclang::AST

#endif
