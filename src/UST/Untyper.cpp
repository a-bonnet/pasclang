#include "UST/Untyper.h"
#include "AST/AST.h"

namespace pasclang::UST {

std::unique_ptr<UST::Node>& Untyper::translate(std::unique_ptr<AST::Program>& ast)
{
    this->lastNode.release();
    ast->accept(*this);
    return this->lastNode;
}

void Untyper::visit(AST::PrimitiveType& type)
{
    this->lastType.kind = (type.getType()->kind == AST::TableOfTypes::TypeKind::Integer ?
            UST::NativeType::Int : UST::NativeType::Bool);
    this->lastType.dimension = type.getType()->dimension;
}

void Untyper::visit(AST::Expression&)
{
}

void Untyper::visit(AST::EConstant&)
{
}

void Untyper::visit(AST::ECBoolean& boolean)
{
    this->lastNode = std::make_unique<UST::ImmediateBool>(boolean.getValue());
}

void Untyper::visit(AST::ECInteger& integer)
{
    this->lastNode = std::make_unique<UST::ImmediateBool>(integer.getValue());
}

void Untyper::visit(AST::EVariableAccess& variable)
{
    this->lastNode = std::make_unique<UST::Load>(variable.getName());
}

void Untyper::visit(AST::EUnaryOperation& operation)
{
    operation.accept(*this);
    std::unique_ptr<UST::Node> node;
    switch(operation.getType())
    {
        case AST::EUnaryOperation::Type::UnaryNot:
            this->lastNode = std::make_unique<UST::NotBool>(this->lastNode);
            break;
        case AST::EUnaryOperation::Type::UnaryMinus:
            node = std::make_unique<UST::ImmediateInt>(0);
            this->lastNode = std::make_unique<UST::SubInt>(
                    node,
                    this->lastNode
                );
    }
}

void Untyper::visit(AST::EBinaryOperation& operation)
{
    operation.getLeft()->accept(*this);
    std::unique_ptr<UST::Node> lhs = std::move(this->lastNode);
    operation.getRight()->accept(*this);

    switch(operation.getType())
    {
        case AST::EBinaryOperation::Type::BinaryAddition:
            this->lastNode = std::make_unique<UST::AddInt>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinarySubtraction:
            this->lastNode = std::make_unique<UST::SubInt>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryMultiplication:
            this->lastNode = std::make_unique<UST::MulInt>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryDivision:
            this->lastNode = std::make_unique<UST::DivInt>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryEquality:
            this->lastNode = std::make_unique<UST::CmpEQ>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryNonEquality:
            this->lastNode = std::make_unique<UST::CmpNEQ>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryLogicalAnd:
            this->lastNode = std::make_unique<UST::AndBool>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryLogicalOr:
            this->lastNode = std::make_unique<UST::OrBool>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryLogicalLessThan:
            this->lastNode = std::make_unique<UST::CmpLT>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryLogicalLessEqual:
            this->lastNode = std::make_unique<UST::CmpLE>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryLogicalGreaterThan:
            this->lastNode = std::make_unique<UST::CmpGT>(lhs, this->lastNode);
            break;
        case AST::EBinaryOperation::Type::BinaryLogicalGreaterEqual:
            this->lastNode = std::make_unique<UST::CmpGE>(lhs, this->lastNode);
            break;
    }
}

void Untyper::visit(AST::EFunctionCall& call)
{
    std::list<std::unique_ptr<UST::Node>> actuals;
    for(std::unique_ptr<AST::Expression>& actual : call.getActuals())
    {
        actual->accept(*this);
        actuals.push_back(std::move(this->lastNode));
    }

    this->lastNode = std::make_unique<UST::Call>(call.getName(), actuals);
}

void Untyper::visit(AST::EArrayAccess& access)
{
    access.getArray()->accept(*this);
    std::unique_ptr<UST::Node> array = std::move(this->lastNode);
    access.getIndex()->accept(*this);
    this->lastNode = std::make_unique<UST::Array>(array, this->lastNode);
}

void Untyper::visit(AST::EArrayAllocation& allocation)
{
    allocation.getType()->accept(*this);
    allocation.getElements()->accept(*this);
    this->lastNode = std::make_unique<UST::Alloc>(this->lastType, this->lastNode);
}

void Untyper::visit(AST::Instruction&)
{
}

void Untyper::visit(AST::IProcedureCall& call)
{
    std::list<std::unique_ptr<UST::Node>> actuals;
    for(std::unique_ptr<AST::Expression>& actual : call.getActuals())
    {
        actual->accept(*this);
        actuals.push_back(std::move(this->lastNode));
    }

    this->lastNode = std::make_unique<UST::Call>(call.getName(), actuals);
}

void Untyper::visit(AST::IVariableAssignment& assignment)
{
    assignment.getValue()->accept(*this);
    this->lastNode = std::make_unique<UST::Store>(assignment.getName(), this->lastNode);
}

void Untyper::visit(AST::IArrayAssignment& assignment)
{
    assignment.getArray()->accept(*this);
    std::unique_ptr<UST::Node> array = std::move(this->lastNode);
    assignment.getIndex()->accept(*this);
    std::unique_ptr<UST::Node> index = std::move(this->lastNode);
    std::unique_ptr<UST::Node> location = std::make_unique<UST::Array>(array, index);

    assignment.getValue()->accept(*this);
    this->lastNode = std::make_unique<UST::StoreAt>(location, this->lastNode);
}

void Untyper::visit(AST::ISequence& sequence)
{
    std::list<std::unique_ptr<UST::Node>> instructions;

    for(std::unique_ptr<AST::Instruction>& instruction : sequence.getInstructions())
    {
        instruction->accept(*this);
        instructions.push_back(std::move(this->lastNode));
    }

    this->lastNode = std::make_unique<UST::Sequence>(instructions);
}

void Untyper::visit(AST::ICondition& condition)
{
    condition.getCondition()->accept(*this);
    std::unique_ptr<UST::Node> conditionNode = std::move(this->lastNode);
    condition.getTrue()->accept(*this);
    std::unique_ptr<UST::Node> branchTrue = std::move(this->lastNode);
    std::unique_ptr<UST::Node> branchFalse(nullptr);

    // TODO: write this a bit better
    if(condition.getFalse().get() == nullptr)
    {
        this->lastNode = std::make_unique<UST::Branch>(conditionNode, branchTrue, branchFalse);
    }
    else
    {
        condition.getFalse()->accept(*this);
        branchFalse = std::move(this->lastNode);
        this->lastNode = std::make_unique<UST::Branch>(conditionNode, branchTrue,
                branchFalse);
    }
}

void Untyper::visit(AST::IRepetition& repetition)
{
    repetition.getCondition()->accept(*this);
    std::unique_ptr<UST::Node> condition = std::move(this->lastNode);
    repetition.getInstructions();

    this->lastNode = std::make_unique<Repeat>(condition, this->lastNode);
}

void Untyper::visit(AST::Procedure& definition)
{
	std::map<std::string, std::unique_ptr<UST::Alloc>> locals;
    std::unique_ptr<UST::Node> body;
}

void Untyper::visit(AST::Program& program)
{
}

} // namespace pasclang::UST

