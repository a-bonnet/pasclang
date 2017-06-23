#ifndef PASCLANG_AST_H
#define PASCLANG_AST_H

#include <map>
#include <list>
#include <string>
#include <memory>

#include "AST/Types.h"
#include "Parsing/Location.h"

namespace pasclang::AST {

class Visitor;

class Node
{
    private:
        std::unique_ptr<Parsing::Location> location;

    public:
        Node(std::unique_ptr<Parsing::Location>& location) : location(std::move(location)) { }
        virtual ~Node() { }
        virtual void accept(Visitor& visitor) = 0;
        const Parsing::Location* getLocation() const { return this->location.get(); }
};

class PrimitiveType;
class Expression;
class EConstant;
class ECBoolean;
class ECInteger;
class EVariableAccess;
class EUnaryOperation;
class EBinaryOperation;
class EFunctionCall;
class EArrayAccess;
class EArrayAllocation;
class Instruction;
class IProcedureCall;
class IVariableAssignment;
class IArrayAssignment;
class ISequence;
class ICondition;
class IRepetition;
class Procedure;
class Program;

class Visitor
{
    public:
        Visitor() { }
        virtual ~Visitor() { }

        virtual void visit(PrimitiveType& type) = 0;
        virtual void visit(Expression& expression) = 0;
        virtual void visit(EConstant& constant) = 0;
        virtual void visit(ECBoolean& boolean) = 0;
        virtual void visit(ECInteger& integer) = 0;
        virtual void visit(EVariableAccess& variable) = 0;
        virtual void visit(EUnaryOperation& operation) = 0;
        virtual void visit(EBinaryOperation& operation) = 0;
        virtual void visit(EFunctionCall& call) = 0;
        virtual void visit(EArrayAccess& access) = 0;
        virtual void visit(EArrayAllocation& allocation) = 0;
        virtual void visit(Instruction& instruction) = 0;
        virtual void visit(IProcedureCall& call) = 0;
        virtual void visit(IVariableAssignment& assignment) = 0;
        virtual void visit(IArrayAssignment& assignment) = 0;
        virtual void visit(ISequence& sequence) = 0;
        virtual void visit(ICondition& condition) = 0;
        virtual void visit(IRepetition& repetition) = 0;
        virtual void visit(Procedure& definition) = 0;
        virtual void visit(Program& program) = 0;
};

typedef std::unique_ptr<Parsing::Location> LocationPtr;

class PrimitiveType : public Node
{
    private:
        const TableOfTypes::Type* type;

    public:
        PrimitiveType(const TableOfTypes::Type* type, LocationPtr& location) :
            Node(location), type(type) { }
        virtual ~PrimitiveType() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        void increaseDimension() { this->type = TableOfTypes::get(this->type->kind, this->type->dimension + 1); }
        const TableOfTypes::Type* getType() const { return this->type; }
};

class Instruction : public Node
{
    public:
        Instruction(LocationPtr& location) : Node(location) { }
        virtual ~Instruction() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Expression : public Node
{
    public:
        Expression(LocationPtr& location) : Node(location) { }
        virtual ~Expression() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class EConstant : public Expression
{
    public:
        EConstant(LocationPtr& location) : Expression(location) { }
        virtual ~EConstant() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class ECBoolean final : public EConstant
{
    private:
        bool value;

    public:
        ECBoolean(bool value, LocationPtr& location) : EConstant(location), value(value) { }
        virtual ~ECBoolean() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        bool getValue() const { return this->value; }
};

class ECInteger final : public EConstant
{
    private:
        std::int32_t value;

    public:
        ECInteger(std::int32_t value, LocationPtr& location) : EConstant(location), value(value) { }
        virtual ~ECInteger() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::int32_t getValue() const { return this->value; }
};

class EVariableAccess final : public Expression
{
    private:
        std::string name;

    public:
        EVariableAccess(std::string& name, LocationPtr& location) : Expression(location), name(name) { }
        virtual ~EVariableAccess() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        const std::string& getName() const { return this->name; }
};

class EUnaryOperation final : public Expression
{
    public:
        enum Type {
            UnaryMinus,
            UnaryNot
        };

    private:
        Type type;
        std::unique_ptr<Expression> expression;

    public:
        EUnaryOperation(Type type, std::unique_ptr<Expression>& expression, LocationPtr& location) :
            Expression(location), type(type), expression(std::move(expression)) { }
        virtual ~EUnaryOperation() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        Type getType() const { return this->type; }
        Expression* getExpression() { return this->expression.get(); }
};

class EBinaryOperation final : public Expression
{
    public:
        enum Type
        {
            BinaryAddition,
            BinarySubtraction,
            BinaryMultiplication,
            BinaryDivision,
            BinaryLogicalLessThan,
            BinaryLogicalLessEqual,
            BinaryLogicalGreaterThan,
            BinaryLogicalGreaterEqual,
            BinaryLogicalOr,
            BinaryLogicalAnd,
            BinaryEquality,
            BinaryNonEquality
        };

    private:
        Type type;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;

    public:
        EBinaryOperation(Type type, std::unique_ptr<Expression>& lhs, std::unique_ptr<Expression>& rhs, LocationPtr& location) :
            Expression(location), type(type), left(std::move(lhs)), right(std::move(rhs)) { }
        virtual ~EBinaryOperation() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        Type getType() const { return this->type; }
        std::unique_ptr<Expression>& getLeft() { return this->left; }
        std::unique_ptr<Expression>& getRight() { return this->right; }
};

class EFunctionCall final : public Expression
{
    private:
        std::string name;
        std::list<std::unique_ptr<Expression>> actuals;

    public:
        EFunctionCall(std::string& name, std::list<std::unique_ptr<Expression>>& actuals, LocationPtr& location) :
            Expression(location), name(name), actuals(std::move(actuals)) { }
        virtual ~EFunctionCall() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::string& getName() { return this->name; }
        std::list<std::unique_ptr<Expression>>& getActuals() { return this->actuals; }
};

class EArrayAccess final : public Expression
{
    private:
        std::unique_ptr<Expression> array;
        std::unique_ptr<Expression> index;

    public:
        EArrayAccess(std::unique_ptr<Expression>& array, std::unique_ptr<Expression>& index, LocationPtr& location) :
            Expression(location), array(std::move(array)), index(std::move(index)) { }
        virtual ~EArrayAccess() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::unique_ptr<Expression>& getArray() { return this->array; }
        std::unique_ptr<Expression>& getIndex() { return this->index; }
};

class EArrayAllocation final : public Expression
{
    private:
        std::unique_ptr<PrimitiveType> type;
        std::unique_ptr<Expression> elements;

    public:
        EArrayAllocation(std::unique_ptr<PrimitiveType>& type, std::unique_ptr<Expression>& elements, LocationPtr& location) :
            Expression(location), type(std::move(type)), elements(std::move(elements)) { }
        virtual ~EArrayAllocation() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::unique_ptr<PrimitiveType>& getType() { return this->type; }
        std::unique_ptr<Expression>& getElements() { return this->elements; }
};

class IProcedureCall final : public Instruction
{
    private:
        std::string name;
        std::list<std::unique_ptr<Expression>> actuals;

    public:
        IProcedureCall(std::string& name, std::list<std::unique_ptr<Expression>>& actuals, LocationPtr& location) :
            Instruction(location), name(name), actuals(std::move(actuals)) { }
        virtual ~IProcedureCall() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::string& getName() { return this->name; }
        std::list<std::unique_ptr<Expression>>& getActuals() { return this->actuals; }
};

class IVariableAssignment final : public Instruction
{
    private:
        std::string name;
        std::unique_ptr<Expression> value;

    public:
        IVariableAssignment(std::string& name, std::unique_ptr<Expression>& expression, LocationPtr& location) :
            Instruction(location), name(name), value(std::move(expression)) { }
        virtual ~IVariableAssignment() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::string& getName() { return this->name; }
        std::unique_ptr<Expression>& getValue() { return this->value; }
};

class IArrayAssignment final : public Instruction
{
    private:
        std::unique_ptr<Expression> array;
        std::unique_ptr<Expression> index;
        std::unique_ptr<Expression> value;

    public:
        IArrayAssignment(std::unique_ptr<Expression>& array,
                std::unique_ptr<Expression>& index,
                std::unique_ptr<Expression>& value, LocationPtr& location) :
            Instruction(location), array(std::move(array)), index(std::move(index)), value(std::move(value)) { }
        virtual ~IArrayAssignment() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::unique_ptr<Expression>& getArray() { return this->array; }
        std::unique_ptr<Expression>& getIndex() { return this->index; }
        std::unique_ptr<Expression>& getValue() { return this->value; }
};

class ISequence final : public Instruction
{
    private:
        std::list<std::unique_ptr<Instruction>> instructions;

    public:
        ISequence(std::list<std::unique_ptr<Instruction>>& instructions, LocationPtr& location) :
            Instruction(location), instructions(std::move(instructions)) { }
        virtual ~ISequence() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::list<std::unique_ptr<Instruction>>& getInstructions() { return this->instructions; }
};

class ICondition final : public Instruction
{
    private:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Instruction> conditionTrue;
        std::unique_ptr<Instruction> conditionFalse;

    public:
        ICondition(std::unique_ptr<Expression>& condition,
                std::unique_ptr<Instruction>& conditionTrue,
                std::unique_ptr<Instruction>& conditionFalse, LocationPtr& location) :
            Instruction(location), condition(std::move(condition)), conditionTrue(std::move(conditionTrue)), conditionFalse(std::move(conditionFalse)) { }
        virtual ~ICondition() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::unique_ptr<Expression>& getCondition() { return this->condition; }
        std::unique_ptr<Instruction>& getTrue() { return this->conditionTrue; }
        std::unique_ptr<Instruction>& getFalse() { return this->conditionFalse; }
};

class IRepetition final : public Instruction
{
    private:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Instruction> instruction;

    public:
        IRepetition(std::unique_ptr<Expression>& condition, std::unique_ptr<Instruction>& instruction, LocationPtr& location) :
            Instruction(location), condition(std::move(condition)), instruction(std::move(instruction)) { }
        virtual ~IRepetition() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::unique_ptr<Expression>& getCondition() { return this->condition; }
        std::unique_ptr<Instruction>& getInstructions() { return this->instruction; }
};

class Procedure final : public Node
{
    private:
        std::string name;
        std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>> formals;
        std::unique_ptr<PrimitiveType> resultType;
        std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>> locals;
        std::unique_ptr<Instruction> body;

    public:
        Procedure(std::string& name,
            std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>& formals,
            std::unique_ptr<PrimitiveType>& resultType,
            std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>& locals,
            std::unique_ptr<Instruction>& body, LocationPtr& location) :
            Node(location), name(std::move(name)),
            formals(std::move(formals)),
            resultType(std::move(resultType)),
            locals(std::move(locals)),
            body(std::move(body)) { }
        virtual ~Procedure() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::string& getName() { return this->name; }
        std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>& getFormals() { return this->formals; }
        std::unique_ptr<PrimitiveType>& getResultType() { return this->resultType; }
        std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>& getLocals() { return this->locals; }
        std::unique_ptr<Instruction>& getBody() { return this->body; }
};

class Program final : public Node
{
    private:
        std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>> globals;
        std::list<std::unique_ptr<Procedure>> procedures;
        std::unique_ptr<Instruction> main;

    public:
        Program(std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>& globals,
            std::list<std::unique_ptr<Procedure>>& procedures,
            std::unique_ptr<Instruction>& main, LocationPtr& location) :
            Node(location), globals(std::move(globals)), procedures(std::move(procedures)), main(std::move(main)) { }
        virtual ~Program() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }

        std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>& getGlobals() { return this->globals; }
        std::list<std::unique_ptr<Procedure>>& getProcedures() { return this->procedures; }
        std::unique_ptr<Instruction>& getMain() { return this->main; }
};

} // namespace pasclang::AST

#endif
