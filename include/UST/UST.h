// Untyped syntax tree, basically the first intermediate representation

#ifndef PASCLANG_UST_H
#define PASCLANG_UST_H

#include <string>
#include <map>
#include <list>
#include <memory>

namespace pasclang::UST {

// Still required to know the size of types later since they're architecture-dependent
enum NativeType {
    Int,
    Bool
};

struct USTType {
    NativeType kind;
    unsigned int dimension;
};

class Visitor;

class Node
{
    public:
        virtual ~Node() { }
        virtual void accept(Visitor& visitor) = 0;
};

typedef std::unique_ptr<UST::Node> NodePtr;

class ImmediateInt;
class ImmediateBool;
class Alloc;
class Load;
class Store;
class StoreAt;
class Array;
class AndBool;
class OrBool;
class NotBool;
class AddInt;
class SubInt;
class MulInt;
class DivInt;
class CmpLT;
class CmpLE;
class CmpGT;
class CmpGE;
class CmpEQ;
class CmpNEQ;
class Call;
class Sequence;
class Branch;
class Repeat;
class Procedure;
class Program;

class Visitor
{
    public:
        Visitor() { }
        virtual ~Visitor() { }

        virtual void visit(ImmediateInt&) = 0;
        virtual void visit(ImmediateBool&) = 0;
        virtual void visit(Alloc&) = 0;
        virtual void visit(Load&) = 0;
        virtual void visit(Store&) = 0;
        virtual void visit(StoreAt&) = 0;
        virtual void visit(Array&) = 0;
        virtual void visit(AndBool&) = 0;
        virtual void visit(OrBool&) = 0;
        virtual void visit(NotBool&) = 0;
        virtual void visit(AddInt&) = 0;
        virtual void visit(SubInt&) = 0;
        virtual void visit(MulInt&) = 0;
        virtual void visit(DivInt&) = 0;
        virtual void visit(CmpLT&) = 0;
        virtual void visit(CmpLE&) = 0;
        virtual void visit(CmpGT&) = 0;
        virtual void visit(CmpGE&) = 0;
        virtual void visit(CmpEQ&) = 0;
        virtual void visit(CmpNEQ&) = 0;
        virtual void visit(Call&) = 0;
        virtual void visit(Sequence&) = 0;
        virtual void visit(Branch&) = 0;
        virtual void visit(Repeat&) = 0;
        virtual void visit(Procedure&) = 0;
        virtual void visit(Program&) = 0;
};

class ImmediateInt : public Node
{
    public:
        int value;
    public:
        ImmediateInt(int value) : value(value) { }
        virtual ~ImmediateInt() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class ImmediateBool : public Node
{
    public:
        bool value;
    public:
        ImmediateBool(bool value) : value(value) { }
        virtual ~ImmediateBool() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Alloc : public Node
{
    public:
        const USTType type;
        NodePtr size;
    public:
        Alloc(USTType type, NodePtr& size) : type(type), size(std::move(size)) { }
        virtual ~Alloc() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Load : public Node
{
    // The loaded symbol
    public:
        const std::string symbol;
    public:
        Load(const std::string& symbol) : symbol(symbol) { }
        virtual ~Load() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Store : public Node
{
    public:
        const std::string symbol;
        NodePtr expression;
    public:
        Store(const std::string& symbol, NodePtr& expression) : symbol(symbol),
            expression(std::move(expression)) { }
        virtual ~Store() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class StoreAt : public Node
{
    public:
        NodePtr location;
        NodePtr expression;
    public:
        StoreAt(NodePtr& location, NodePtr& expression) : location(std::move(location)),
            expression(std::move(expression)) { }
        virtual ~StoreAt() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Array : public Node
{
    public:
        NodePtr array;
        NodePtr index;
    public:
        Array(NodePtr& array, NodePtr& index) : array(std::move(array)),
            index(std::move(index)) { }
        virtual ~Array() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class AndBool : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        AndBool(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~AndBool() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class OrBool : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        OrBool(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~OrBool() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class NotBool : public Node
{
    public:
        NodePtr expression;
    public:
        NotBool(NodePtr& expression) : expression(std::move(expression)) { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class AddInt : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        AddInt(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~AddInt() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class SubInt : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        SubInt(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~SubInt() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class MulInt : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        MulInt(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~MulInt() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class DivInt : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        DivInt(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~DivInt() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class CmpLT : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        CmpLT(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~CmpLT() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class CmpLE : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        CmpLE(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~CmpLE() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class CmpGT : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        CmpGT(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~CmpGT() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class CmpGE : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        CmpGE(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~CmpGE() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class CmpEQ : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        CmpEQ(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~CmpEQ() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class CmpNEQ : public Node
{
    public:
        NodePtr lhs;
        NodePtr rhs;
    public:
        CmpNEQ(NodePtr& lhs, NodePtr& rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) { }
        virtual ~CmpNEQ() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

// It is not certain yet that we need to split procedure and function calls, this is mostly influenced by language semantics
// The distinction will probably arise naturally by having functions bound to new SSA registers unlike procedures
class Call : public Node
{
    public:
        const std::string callee;
        std::list<NodePtr> actuals;
    public:
        Call(std::string& callee, std::list<NodePtr>& actuals) : callee(callee), actuals(std::move(actuals)) { }
        virtual ~Call() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Sequence : public Node
{
    public:
        std::list<NodePtr> sequence;
    public:
        Sequence(std::list<NodePtr>& sequence) : sequence(std::move(sequence)) { }
        virtual ~Sequence() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Branch : public Node
{
    public:
        NodePtr condition;
        NodePtr branchTrue;
        NodePtr branchFalse;
    public:
        Branch(NodePtr& condition, NodePtr& branchTrue, NodePtr& branchFalse)
            : condition(std::move(condition)), branchTrue(std::move(branchTrue)), branchFalse(std::move(branchFalse)) { }
        virtual ~Branch() { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Repeat : public Node
{
    public:
        NodePtr condition;
        NodePtr loop;
    public:
        Repeat(NodePtr& condition, NodePtr& loop) :
            condition(std::move(condition)), loop(std::move(loop)) { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Procedure : public Node
{
    public:
        // Table of symbol and program pointer to refer to global symbols
        const Program* program;
        std::map<std::string, std::unique_ptr<Alloc>> symbols;
        NodePtr body;
    public:
        Procedure(std::map<std::string, std::unique_ptr<Alloc>> symbols,
                NodePtr& body) : symbols(std::move(symbols)), body(std::move(body)) { }
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

class Program : public Node
{
    public:
        std::map<std::string, std::unique_ptr<Alloc>> globals;
        // Note: the main function is stored as a normal procedure, the linker will know what's up
        std::map<std::string, std::unique_ptr<Procedure>> procedures;
    public:
        virtual void accept(Visitor& visitor) override { visitor.visit(*this); }
};

} // namespace pasclang::UST

#endif

