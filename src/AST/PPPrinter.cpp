#include "AST/PPPrinter.h"
#include <iostream>

namespace pasclang::AST {

void PPPrinter::print(std::unique_ptr<AST::Program>& program) const {
    this->buffer.str(std::string());
    this->buffer.clear();
    program->accept(*this);
    std::cout << this->buffer.str() << std::endl;
}

void PPPrinter::indent() const {
    for (size_t i = 0; i < this->indentation; i++)
        this->buffer << "    ";
}

void PPPrinter::visit(const PrimitiveType& type) const {
    for (unsigned i = 0; i < type.getType()->dimension; i++)
        this->buffer << "array of ";

    switch (type.getType()->kind) {
    case AST::TableOfTypes::Boolean:
        this->buffer << "bool";
        break;
    case AST::TableOfTypes::Integer:
        this->buffer << "int";
        break;
    }
}

void PPPrinter::visit(const Instruction&) const {}

void PPPrinter::visit(const Expression&) const {}

void PPPrinter::visit(const EConstant&) const {}

void PPPrinter::visit(const ECBoolean& boolean) const {
    this->buffer << (boolean.getValue() ? "true" : "false");
}

void PPPrinter::visit(const ECInteger& integer) const {
    this->buffer << integer.getValue();
}

void PPPrinter::visit(const EVariableAccess& variable) const {
    this->buffer << variable.getName();
}

void PPPrinter::visit(const EUnaryOperation& operation) const {
    this->buffer << "(";
    switch (operation.getType()) {
    case EUnaryOperation::Type::UnaryMinus:
        this->buffer << "-";
        break;
    case EUnaryOperation::Type::UnaryNot:
        this->buffer << "not ";
        break;
    }
    operation.getExpression().accept(*this);
    this->buffer << ")";
}

void PPPrinter::visit(const EBinaryOperation& operation) const {
    this->buffer << "(";
    operation.getLeft().accept(*this);

    switch (operation.getType()) {
    case EBinaryOperation::Type::BinaryAddition:
        this->buffer << " + ";
        break;
    case EBinaryOperation::Type::BinarySubtraction:
        this->buffer << " - ";
        break;
    case EBinaryOperation::Type::BinaryMultiplication:
        this->buffer << " * ";
        break;
    case EBinaryOperation::Type::BinaryDivision:
        this->buffer << " / ";
        break;
    case EBinaryOperation::Type::BinaryLogicalLessThan:
        this->buffer << " < ";
        break;
    case EBinaryOperation::Type::BinaryLogicalLessEqual:
        this->buffer << " <= ";
        break;
    case EBinaryOperation::Type::BinaryLogicalGreaterThan:
        this->buffer << " > ";
        break;
    case EBinaryOperation::Type::BinaryLogicalGreaterEqual:
        this->buffer << " >= ";
        break;
    case EBinaryOperation::Type::BinaryLogicalOr:
        this->buffer << " or ";
        break;
    case EBinaryOperation::Type::BinaryLogicalAnd:
        this->buffer << " and ";
        break;
    case EBinaryOperation::Type::BinaryEquality:
        this->buffer << " == ";
        break;
    case EBinaryOperation::Type::BinaryNonEquality:
        this->buffer << " <> ";
        break;
    }

    operation.getRight().accept(*this);
    this->buffer << ")";
}

void PPPrinter::visit(const EFunctionCall& call) const {
    this->buffer << call.getName() << "(";

    for (const auto actual : call.getActuals()) {
        actual->accept(*this);
        if (actual != call.getActuals().back())
            this->buffer << ", ";
    }
    this->buffer << ")";
}

void PPPrinter::visit(const EArrayAccess& access) const {
    access.getArray().accept(*this);
    this->buffer << "[";
    access.getIndex().accept(*this);
    this->buffer << "]";
}

void PPPrinter::visit(const EArrayAllocation& allocation) const {
    this->buffer << "new ";
    allocation.getType().accept(*this);
    this->buffer << "[";
    allocation.getElements().accept(*this);
    this->buffer << "]";
}

void PPPrinter::visit(const IProcedureCall& call) const {
    this->indent();
    this->buffer << call.getName() << "(";

    for (auto& actual : call.getActuals()) {
        actual->accept(*this);
        if (actual != call.getActuals().back())
            this->buffer << ", ";
    }
    this->buffer << ")";
}

void PPPrinter::visit(const IVariableAssignment& assignment) const {
    this->indent();
    this->buffer << assignment.getName() << " := ";
    assignment.getValue().accept(*this);
}

void PPPrinter::visit(const IArrayAssignment& assignment) const {
    this->indent();
    assignment.getArray().accept(*this);
    this->buffer << " := ";
    assignment.getValue().accept(*this);
}

void PPPrinter::visit(const ISequence& sequence) const {
    this->indent();
    this->buffer << "begin\n";
    this->indentation++;
    for (const auto instruction : sequence.getInstructions()) {
        instruction->accept(*this);
        if (instruction != sequence.getInstructions().back())
            this->buffer << ";\n";
    }
    this->buffer << "\n";
    this->indentation--;
    this->indent();
    this->buffer << "end";
}

void PPPrinter::visit(const ICondition& condition) const {
    this->indent();
    this->buffer << "if ";
    condition.getCondition().accept(*this);
    this->buffer << " then \n";
    this->indentation++;
    condition.getTrue().accept(*this);
    this->indentation--;
    if (condition.getFalse() != nullptr) {
        this->buffer << "\n";
        this->indent();
        this->buffer << "else \n";
        this->indentation++;
        condition.getFalse()->accept(*this);
        this->indentation--;
    }
}

void PPPrinter::visit(const IRepetition& repetition) const {
    this->indent();
    this->buffer << "while ";
    repetition.getCondition().accept(*this);
    this->buffer << " do\n";
    this->indentation++;
    repetition.getInstructions().accept(*this);
    this->indentation--;
}

void PPPrinter::visit(const Procedure& procedure) const {
    bool isFunction = (procedure.getResultType() != nullptr);
    this->buffer << (isFunction ? "function " : "procedure ")
                 << procedure.getName() << "(";

    auto formals = procedure.getFormals();
    if (!formals.empty()) {
        for (auto& f : formals) {
            this->buffer << f.first << " : ";
            f.second->accept(*this);
            if (f != formals.back())
                this->buffer << " ; ";
        }
    }

    this->buffer << ")";

    if (isFunction) {
        this->buffer << " : ";
        procedure.getResultType()->accept(*this);
    }

    this->buffer << ";\n";

    if (!procedure.getLocals().empty()) {
        this->buffer << "var\n";
        this->indentation++;
        for (auto& global : procedure.getLocals()) {
            this->indent();
            this->buffer << global.first << " : ";
            global.second->accept(*this);
            this->buffer << ";\n";
        }
        this->indentation--;
    }

    procedure.getBody().accept(*this);
    this->buffer << ";\n";
}

void PPPrinter::visit(const Program& program) const {
    this->buffer << "program\n";

    if (!program.getGlobals().empty()) {
        this->buffer << "var\n";
        this->indentation++;
        for (auto& global : program.getGlobals()) {
            this->indent();
            this->buffer << global.first << " : ";
            global.second->accept(*this);
            this->buffer << ";\n";
        }
        this->indentation--;
    }
    this->buffer << '\n';

    for (auto& program : program.getProcedures())
        program->accept(*this);

    program.getMain().accept(*this);

    this->buffer << ".\n";
}

} // namespace pasclang::AST
