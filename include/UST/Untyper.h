#include "AST/AST.h"
#include "UST/UST.h"

namespace pasclang::UST {

class Untyper final : public AST::Visitor
{
    private:
        std::unique_ptr<UST::Node> lastNode;
        USTType lastType;

    public:
        Untyper() { }
        virtual ~Untyper() { }
        std::unique_ptr<UST::Node>& translate(std::unique_ptr<AST::Program>& ast);

        void visit(AST::PrimitiveType& type) override;
        void visit(AST::Expression& expression) override;
        void visit(AST::EConstant& constant) override;
        void visit(AST::ECBoolean& boolean) override;
        void visit(AST::ECInteger& integer) override;
        void visit(AST::EVariableAccess& variable) override;
        void visit(AST::EUnaryOperation& operation) override;
        void visit(AST::EBinaryOperation& operation) override;
        void visit(AST::EFunctionCall& call) override;
        void visit(AST::EArrayAccess& access) override;
        void visit(AST::EArrayAllocation& allocation) override;
        void visit(AST::Instruction& instruction) override;
        void visit(AST::IProcedureCall& call) override;
        void visit(AST::IVariableAssignment& assignment) override;
        void visit(AST::IArrayAssignment& assignment) override;
        void visit(AST::ISequence& sequence) override;
        void visit(AST::ICondition& condition) override;
        void visit(AST::IRepetition& repetition) override;
        void visit(AST::Procedure& definition) override;
        void visit(AST::Program& program) override;
};

} // namespace pasclang::UST

