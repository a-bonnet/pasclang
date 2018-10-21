// This file contains AST definitions and operations on nodes as well as the
// skeleton for AST visitors. AST visitors are designed as builders: they have
// an internal state which they modify as they visit the program tree. Then one
// can access the result with a specific method.

#ifndef PASCLANG_AST_H
#define PASCLANG_AST_H

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "AST/Types.h"
#include "Parsing/Location.h"

namespace pasclang::AST {

class Visitor;

class Node {
  private:
    std::unique_ptr<Parsing::Location> location;

  public:
    Node(std::unique_ptr<Parsing::Location>& location)
        : location(std::move(location)) {}
    virtual ~Node() {}
    virtual void accept(const Visitor& visitor) const = 0;
    const Parsing::Location* getLocation() const {
        return this->location.get();
    }
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

class Visitor {
  public:
    Visitor() {}
    virtual ~Visitor() {}

    virtual void visit(const PrimitiveType& type) const = 0;
    virtual void visit(const Expression& expression) const = 0;
    virtual void visit(const EConstant& constant) const = 0;
    virtual void visit(const ECBoolean& boolean) const = 0;
    virtual void visit(const ECInteger& integer) const = 0;
    virtual void visit(const EVariableAccess& variable) const = 0;
    virtual void visit(const EUnaryOperation& operation) const = 0;
    virtual void visit(const EBinaryOperation& operation) const = 0;
    virtual void visit(const EFunctionCall& call) const = 0;
    virtual void visit(const EArrayAccess& access) const = 0;
    virtual void visit(const EArrayAllocation& allocation) const = 0;
    virtual void visit(const Instruction& instruction) const = 0;
    virtual void visit(const IProcedureCall& call) const = 0;
    virtual void visit(const IVariableAssignment& assignment) const = 0;
    virtual void visit(const IArrayAssignment& assignment) const = 0;
    virtual void visit(const ISequence& sequence) const = 0;
    virtual void visit(const ICondition& condition) const = 0;
    virtual void visit(const IRepetition& repetition) const = 0;
    virtual void visit(const Procedure& definition) const = 0;
    virtual void visit(const Program& program) const = 0;
};

typedef std::unique_ptr<Parsing::Location> LocationPtr;

// Points to a (shared) structure holding type information
class PrimitiveType : public Node {
  private:
    const TableOfTypes::Type* type;

  public:
    PrimitiveType(const TableOfTypes::Type* type, LocationPtr& location)
        : Node(location), type(type) {}
    virtual ~PrimitiveType() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    void increaseDimension() { this->type = this->type->increaseDimension(); }
    const TableOfTypes::Type* getType() const { return this->type; }
};

// Instruction class name begins with 'I'. An instruction has no value.
class Instruction : public Node {
  public:
    Instruction(LocationPtr& location) : Node(location) {}
    virtual ~Instruction() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }
};

// Expression class name begins with 'E'. An expression has a value.
class Expression : public Node {
  public:
    Expression(LocationPtr& location) : Node(location) {}
    virtual ~Expression() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }
};

// A constant literal. Value is the one held by the subclass.
class EConstant : public Expression {
  public:
    EConstant(LocationPtr& location) : Expression(location) {}
    virtual ~EConstant() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }
};

// A boolean being true or false. Value is the boolean.
class ECBoolean final : public EConstant {
  private:
    bool value;

  public:
    ECBoolean(bool value, LocationPtr& location)
        : EConstant(location), value(value) {}
    virtual ~ECBoolean() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    bool getValue() const { return this->value; }
};

// A 32-bits signed integer. Value is the integer.
class ECInteger final : public EConstant {
  private:
    std::int32_t value;

  public:
    ECInteger(std::int32_t value, LocationPtr& location)
        : EConstant(location), value(value) {}
    virtual ~ECInteger() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    std::int32_t getValue() const { return this->value; }
};

// Value is the variable's current value accessed from the table of symbols
class EVariableAccess final : public Expression {
  private:
    std::string name;

  public:
    EVariableAccess(std::string& name, LocationPtr& location)
        : Expression(location), name(name) {}
    virtual ~EVariableAccess() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getName() const { return this->name; }
};

// Evaluates the expression, value is the operation's result.
class EUnaryOperation final : public Expression {
  public:
    enum class Type { UnaryMinus, UnaryNot };

  private:
    Type type;
    std::unique_ptr<Expression> expression;

  public:
    EUnaryOperation(Type type, std::unique_ptr<Expression>& expression,
                    LocationPtr& location)
        : Expression(location), type(type), expression(std::move(expression)) {}
    virtual ~EUnaryOperation() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    Type getType() const { return this->type; }
    const Expression& getExpression() const { return *this->expression.get(); }
};

// Evaluates left then right-hand side, value is the computed operation's
// result.
class EBinaryOperation final : public Expression {
  public:
    enum class Type {
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
    EBinaryOperation(Type type, std::unique_ptr<Expression>& lhs,
                     std::unique_ptr<Expression>& rhs, LocationPtr& location)
        : Expression(location), type(type), left(std::move(lhs)),
          right(std::move(rhs)) {}
    virtual ~EBinaryOperation() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    Type getType() const { return this->type; }
    const Expression& getLeft() const { return *this->left.get(); }
    const Expression& getRight() const { return *this->right.get(); }
};

// Evaluates all arguments from left to right, value is the function's returned
// value.
class EFunctionCall final : public Expression {
  private:
    std::string name;
    std::list<std::unique_ptr<Expression>> actuals;

  public:
    EFunctionCall(std::string& name,
                  std::list<std::unique_ptr<Expression>>& actuals,
                  LocationPtr& location)
        : Expression(location), name(name), actuals(std::move(actuals)) {}
    virtual ~EFunctionCall() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getName() const { return this->name; }
    const std::vector<const Expression*> getActuals() const {
        std::vector<const Expression*> result;

        for (const auto& actual : actuals)
            result.push_back(actual.get());

        return result;
    }
};

// Computes the array address then the index, returns the value stored at the
// computed location.
class EArrayAccess final : public Expression {
  private:
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> index;

  public:
    EArrayAccess(std::unique_ptr<Expression>& array,
                 std::unique_ptr<Expression>& index, LocationPtr& location)
        : Expression(location), array(std::move(array)),
          index(std::move(index)) {}
    virtual ~EArrayAccess() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const Expression& getArray() const { return *this->array.get(); }
    const Expression& getIndex() const { return *this->index.get(); }
};

// Allocates a new (eventually multi-dimensional) array. Value is the allocated
// block's starting location.
class EArrayAllocation final : public Expression {
  private:
    std::unique_ptr<PrimitiveType> type;
    std::unique_ptr<Expression> elements;

  public:
    EArrayAllocation(std::unique_ptr<PrimitiveType>& type,
                     std::unique_ptr<Expression>& elements,
                     LocationPtr& location)
        : Expression(location), type(std::move(type)),
          elements(std::move(elements)) {}
    virtual ~EArrayAllocation() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const PrimitiveType& getType() const { return *this->type.get(); }
    const Expression& getElements() const { return *this->elements.get(); }
};

// Evaluates from left to right.
class IProcedureCall final : public Instruction {
  private:
    std::string name;
    std::list<std::unique_ptr<Expression>> actuals;

  public:
    IProcedureCall(std::string& name,
                   std::list<std::unique_ptr<Expression>>& actuals,
                   LocationPtr& location)
        : Instruction(location), name(name), actuals(std::move(actuals)) {}
    virtual ~IProcedureCall() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getName() const { return this->name; }
    const std::vector<const Expression*> getActuals() const {
        std::vector<const Expression*> result;

        for (const auto& actual : actuals)
            result.push_back(actual.get());

        return result;
    }
};

// Stores the computed value at the variable's address.
class IVariableAssignment final : public Instruction {
  private:
    std::string name;
    std::unique_ptr<Expression> value;

  public:
    IVariableAssignment(std::string& name,
                        std::unique_ptr<Expression>& expression,
                        LocationPtr& location)
        : Instruction(location), name(name), value(std::move(expression)) {}
    virtual ~IVariableAssignment() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getName() const { return this->name; }
    const Expression& getValue() const { return *this->value.get(); }
};

// Computes the array address, the index and uses the resulting address to store
// the computed value.
class IArrayAssignment final : public Instruction {
  private:
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> value;

  public:
    IArrayAssignment(std::unique_ptr<Expression>& array,
                     std::unique_ptr<Expression>& value, LocationPtr& location)
        : Instruction(location), array(std::move(array)),
          value(std::move(value)) {}
    virtual ~IArrayAssignment() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const Expression& getArray() const { return *this->array.get(); }
    const Expression& getValue() const { return *this->value.get(); }
};

// Executes each instruction in order.
class ISequence final : public Instruction {
  private:
    std::list<std::unique_ptr<Instruction>> instructions;

  public:
    ISequence(std::list<std::unique_ptr<Instruction>>& instructions,
              LocationPtr& location)
        : Instruction(location), instructions(std::move(instructions)) {}
    virtual ~ISequence() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::vector<const Instruction*> getInstructions() const {
        std::vector<const Instruction*> result;

        for (const auto& instruction : instructions)
            result.push_back(instruction.get());

        return result;
    }
};

// Evaluates the boolean condition. If value is true, executes conditionTrue,
// else if false and conditionFalse exists, executes conditionFalse.
class ICondition final : public Instruction {
  private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Instruction> conditionTrue;
    std::unique_ptr<Instruction> conditionFalse;

  public:
    ICondition(std::unique_ptr<Expression>& condition,
               std::unique_ptr<Instruction>& conditionTrue,
               std::unique_ptr<Instruction>& conditionFalse,
               LocationPtr& location)
        : Instruction(location), condition(std::move(condition)),
          conditionTrue(std::move(conditionTrue)),
          conditionFalse(std::move(conditionFalse)) {}
    virtual ~ICondition() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const Expression& getCondition() const { return *this->condition.get(); }
    const Instruction& getTrue() const { return *this->conditionTrue.get(); }
    const Instruction* getFalse() const { return this->conditionFalse.get(); }
};

// Computes the boolean condition's value, repeats evaluating the instruction as
// long as the condition is true.
class IRepetition final : public Instruction {
  private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Instruction> instruction;

  public:
    IRepetition(std::unique_ptr<Expression>& condition,
                std::unique_ptr<Instruction>& instruction,
                LocationPtr& location)
        : Instruction(location), condition(std::move(condition)),
          instruction(std::move(instruction)) {}
    virtual ~IRepetition() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const Expression& getCondition() const { return *this->condition.get(); }
    const Instruction& getInstructions() const {
        return *this->instruction.get();
    }
};

// A function or a procedure.
class Procedure final : public Node {
  private:
    std::string name;
    std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>> formals;
    std::unique_ptr<PrimitiveType> resultType;
    std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>> locals;
    std::unique_ptr<Instruction> body;

  public:
    Procedure(std::string& name,
              std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>&
                  formals,
              std::unique_ptr<PrimitiveType>& resultType,
              std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>&
                  locals,
              std::unique_ptr<Instruction>& body, LocationPtr& location)
        : Node(location), name(std::move(name)), formals(std::move(formals)),
          resultType(std::move(resultType)), locals(std::move(locals)),
          body(std::move(body)) {}
    virtual ~Procedure() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getName() const { return this->name; }

    std::vector<std::pair<std::string, const PrimitiveType*>>
    getFormals() const {
        std::vector<std::pair<std::string, const PrimitiveType*>> result;

        for (auto& [name, type] : this->formals) {
            result.push_back(std::make_pair(name, type.get()));
        }

        return result;
    }

    const PrimitiveType* getResultType() const {
        return this->resultType.get();
    }

    const std::vector<std::pair<std::string, const PrimitiveType*>>
    getLocals() const {
        std::vector<std::pair<std::string, const PrimitiveType*>> result;

        for (const auto& [name, type] : this->locals)
            result.push_back(std::make_pair(name, type.get()));

        return result;
    }

    const Instruction& getBody() const { return *this->body.get(); }
};

// A program
class Program final : public Node {
  private:
    std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>> globals;
    std::list<std::unique_ptr<Procedure>> procedures;
    std::unique_ptr<Instruction> main;
    std::unique_ptr<TableOfTypes> tot;

  public:
    Program(std::list<std::pair<std::string, std::unique_ptr<PrimitiveType>>>&
                globals,
            std::list<std::unique_ptr<Procedure>>& procedures,
            std::unique_ptr<Instruction>& main, LocationPtr& location,
            std::unique_ptr<TableOfTypes>& table)
        : Node(location), globals(std::move(globals)),
          procedures(std::move(procedures)), main(std::move(main)),
          tot(std::move(table)) {}
    virtual ~Program() {}
    virtual void accept(const Visitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::vector<std::pair<std::string, const PrimitiveType*>>
    getGlobals() const {
        std::vector<std::pair<std::string, const PrimitiveType*>> result;

        for (auto& [name, type] : this->globals) {
            result.push_back(std::make_pair(name, type.get()));
        }

        return result;
    }

    const std::vector<const Procedure*> getProcedures() const {
        std::vector<const Procedure*> result;

        for (auto& procedure : this->procedures)
            result.push_back(procedure.get());

        return result;
    }
    const Instruction& getMain() const { return *this->main.get(); }
    const TableOfTypes& getTypes() const { return *this->tot.get(); }
};

} // namespace pasclang::AST

#endif
