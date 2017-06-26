#include "AST/PPPrinter.h"
#include <iostream>

namespace pasclang::AST {

void PPPrinter::print(std::unique_ptr<AST::Program>& program)
{
    this->buffer.clear();
    program->accept(*this);
    std::cout << this->buffer << std::endl;
}

void PPPrinter::indent()
{
    for(size_t i = 0 ; i < this->indentation ; i++)
        this->buffer += "    ";
}

void PPPrinter::visit(PrimitiveType& type)
{
    for(unsigned i = 0 ; i < type.getType()->dimension ; i++)
        this->buffer += "array of ";

    switch(type.getType()->kind)
    {
        case AST::TableOfTypes::Boolean:
            this->buffer += "bool";
            break;
        case AST::TableOfTypes::Integer:
            this->buffer += "int";
            break;
    }
}

void PPPrinter::visit(Instruction&)
{
}

void PPPrinter::visit(Expression&)
{
}

void PPPrinter::visit(EConstant&)
{
}

void PPPrinter::visit(ECBoolean& boolean)
{
    boolean.getValue() ? this->buffer += "true" : this->buffer += "false";
}

void PPPrinter::visit(ECInteger& integer)
{
    this->buffer += std::to_string(integer.getValue());
}

void PPPrinter::visit(EVariableAccess& variable)
{
    this->buffer += variable.getName();
}

void PPPrinter::visit(EUnaryOperation& operation)
{
    this->buffer += "(";
    switch(operation.getType()) {
        case EUnaryOperation::Type::UnaryMinus:
            this->buffer += "-";
            break;
        case EUnaryOperation::Type::UnaryNot:
            this->buffer += "not ";
            break;
    }
    operation.getExpression()->accept(*this);
    this->buffer += ")";
}

void PPPrinter::visit(EBinaryOperation& operation)
{
    this->buffer += "(";
    operation.getLeft()->accept(*this);

    switch(operation.getType()) {
        case EBinaryOperation::Type::BinaryAddition:
            this->buffer += " + ";
            break;
        case EBinaryOperation::Type::BinarySubtraction:
            this->buffer += " - ";
            break;
        case EBinaryOperation::Type::BinaryMultiplication:
            this->buffer += " * ";
            break;
        case EBinaryOperation::Type::BinaryDivision:
            this->buffer += " / ";
            break;
        case EBinaryOperation::Type::BinaryLogicalLessThan:
            this->buffer += " < ";
            break;
        case EBinaryOperation::Type::BinaryLogicalLessEqual:
            this->buffer += " <= ";
            break;
        case EBinaryOperation::Type::BinaryLogicalGreaterThan:
            this->buffer += " > ";
            break;
        case EBinaryOperation::Type::BinaryLogicalGreaterEqual:
            this->buffer += " >= ";
            break;
        case EBinaryOperation::Type::BinaryLogicalOr:
            this->buffer += " or ";
        case EBinaryOperation::Type::BinaryLogicalAnd:
            this->buffer += " and ";
            break;
        case EBinaryOperation::Type::BinaryEquality:
            this->buffer += " == ";
            break;
        case EBinaryOperation::Type::BinaryNonEquality:
            this->buffer += " <> ";
            break;
    }

    operation.getRight()->accept(*this);
    this->buffer += ")";
}

void PPPrinter::visit(EFunctionCall& call)
{
    this->buffer += call.getName() + "(";

    for(auto& actual: call.getActuals())
    {
        actual->accept(*this);
        if(actual != call.getActuals().back())
            this->buffer += ", ";
    }
    this->buffer += ")";
}

void PPPrinter::visit(EArrayAccess& access)
{
    access.getArray()->accept(*this);
    this->buffer += "[";
    access.getIndex()->accept(*this);
    this->buffer += "]";
}

void PPPrinter::visit(EArrayAllocation& allocation)
{
    this->buffer += "new ";
    allocation.getType()->accept(*this);
    this->buffer += "[";
    allocation.getElements()->accept(*this);
    this->buffer += "]";
}

void PPPrinter::visit(IProcedureCall& call)
{
    this->indent();
    this->buffer += call.getName() + "(";

    for(auto& actual: call.getActuals())
    {
        actual->accept(*this);
        if(actual != call.getActuals().back())
            this->buffer += ", ";
    }
    this->buffer += ")";
}

void PPPrinter::visit(IVariableAssignment& assignment)
{
    this->indent();
    this->buffer += assignment.getName() + " := ";
    assignment.getValue()->accept(*this);
}

void PPPrinter::visit(IArrayAssignment& assignment)
{
    this->indent();
    assignment.getArray()->accept(*this);
    this->buffer += "[";
    assignment.getIndex()->accept(*this);
    this->buffer += "] := ";
    assignment.getValue()->accept(*this);
}

void PPPrinter::visit(ISequence& sequence)
{
    this->indent();
    this->buffer += "begin\n";
    this->indentation++;
    for(std::unique_ptr<Instruction>& instruction : sequence.getInstructions())
    {
        instruction->accept(*this);
        if(instruction != sequence.getInstructions().back())
            this->buffer += ";\n";
    }
    this->buffer +="\n";
    this->indentation--;
    this->indent();
    this->buffer += "end";
}

void PPPrinter::visit(ICondition& condition)
{
    this->indent();
    this->buffer += "if ";
    condition.getCondition()->accept(*this);
    this->buffer += " then \n";
    this->indentation++;
    condition.getTrue()->accept(*this);
    this->indentation--;
    if(condition.getFalse() != nullptr)
    {
        this->buffer += "\n";
        this->indent();
        this->buffer += "else \n";
        this->indentation++;
        condition.getFalse()->accept(*this);
        this->indentation--;
    }
}

void PPPrinter::visit(IRepetition& repetition)
{
    this->indent();
    this->buffer += "while ";
    repetition.getCondition()->accept(*this);
    this->buffer += " do\n";
    this->indentation++;
    repetition.getInstructions()->accept(*this);
    this->indentation--;
}

void PPPrinter::visit(Procedure& procedure)
{
    bool isFunction = (procedure.getResultType() != nullptr);
    this->buffer +=
        (isFunction ? "function " : "procedure ")
         + procedure.getName() + "(";

    if(!procedure.getFormals().empty())
    {
        unsigned nIter = 0;
        for(auto iter = procedure.getFormals().begin() ; iter != procedure.getFormals().end() ; ++iter)
        {
            this->buffer += iter->first + " : ";
            iter->second->accept(*this);
            if(nIter < procedure.getFormals().size() - 1)
                this->buffer += " ; ";
            nIter++;
        }
    }

    this->buffer += ")";

    if(isFunction)
    {
        this->buffer += " : ";
        procedure.getResultType()->accept(*this);
    }

    this->buffer += ";\n";

    if(!procedure.getLocals().empty())
    {
        this->buffer += "var\n";
        this->indentation++;
        for(auto& global : procedure.getLocals())
        {
            this->indent();
            this->buffer += global.first + " : ";
            global.second->accept(*this);
            this->buffer += ";\n";
        }
        this->indentation--;
    }

    procedure.getBody()->accept(*this);
    this->buffer += ";\n";
}

void PPPrinter::visit(Program& program)
{
    this->buffer += "program\n";

    if(!program.getGlobals().empty())
    {
        this->buffer += "var\n";
        this->indentation++;
        for(auto& global : program.getGlobals())
        {
            this->indent();
            this->buffer += global.first + " : ";
            global.second->accept(*this);
            this->buffer += ";\n";
        }
        this->indentation--;
    }
    this->buffer += '\n';

    for(auto& program : program.getProcedures())
        program->accept(*this);

    program.getMain()->accept(*this);

    this->buffer += ".\n";
}

}

