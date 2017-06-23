#ifndef PASCLANG_AST_PRINTER_H
#define PASCLANG_AST_PRINTER_H

#include "AST/AST.h"
#include <iostream>

namespace pasclang::AST {

class PPPrinter : public Visitor
{
    private:
        size_t indentation = 0;
        std::string buffer = "";

    public:
        PPPrinter() { }
        virtual ~PPPrinter() { }
        void print(std::unique_ptr<AST::Program>& program);
        void indent();

        virtual void visit(PrimitiveType& type);
        virtual void visit(Expression& expression);
        virtual void visit(EConstant& constant);
        virtual void visit(ECBoolean& boolean);
        virtual void visit(ECInteger& integer);
        virtual void visit(EVariableAccess& variable);
        virtual void visit(EUnaryOperation& operation);
        virtual void visit(EBinaryOperation& operation);
        virtual void visit(EFunctionCall& call);
        virtual void visit(EArrayAccess& access);
        virtual void visit(EArrayAllocation& allocation);
        virtual void visit(Instruction& instruction);
        virtual void visit(IProcedureCall& call);
        virtual void visit(IVariableAssignment& assignment);
        virtual void visit(IArrayAssignment& assignment);
        virtual void visit(ISequence& sequence);
        virtual void visit(ICondition& condition);
        virtual void visit(IRepetition& repetition);
        virtual void visit(Procedure& definition);
        virtual void visit(Program& program);
};

} // namespace pasclang::AST

#endif

