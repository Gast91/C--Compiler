#pragma once
#include <string>
#include <optional>
#include <map>
#include <vector>

#include "Visitor.h"

#define OPTIMIZE_TEMPS  // This global somehow?
// Optimization flag enables the recycling of already processed temporary variables
// and prevents -> _t0 = a * b -> c = _t0 and instead optimizes to c = a * b.
#ifdef OPTIMIZE_TEMPS
#define fetch_instr(x) Temporary::CheckAndRecycle(GetValue(x))
#else
#define fetch_instr(x) GetValue(x)
#endif // OPTIMIZE_TEMPS

enum class CmdType { ARITHM, RELAT, UNARY, IF, LABEL, GOTO, COPY, RET, REG, NONE };
struct Command
{
    std::string value;
    CmdType type;
};

struct Operand
{
    CmdType type;
    std::string name;
    std::string address;
};

struct Quadruples
{
    std::optional<Command> op;
    std::optional<Operand> src1;
    std::optional<Operand> src2;
    std::optional<Operand> dest;
};

class Temporary
{
private:
    static int tempCount;
public:
    static const Operand NewTemporary() { return Operand{ CmdType::REG, "_t" + std::to_string(tempCount), "_t" + std::to_string(tempCount++) }; } // address? const?
    // If a temporary is passed to it, it drops the counter effectively recycling that temporary
    // This should never be called by itself and rather through the obtain_source macro. I know bad design...
    static const Quadruples CheckAndRecycle(const Quadruples& potentialTemporary)
    { 
        // What if an identifier starting with _t is passed to it...
        if (potentialTemporary.dest.value().type == CmdType::REG) --tempCount;
        return potentialTemporary;
    }
};

class Label
{
private:
    static int labelCount;
public:
    static const Operand NewLabel() { return Operand{ CmdType::LABEL, "_L" + std::to_string(labelCount), "_L" + std::to_string(labelCount++) }; }
};

// CodeGenerator derives from ValueGetter by the 'Curiously Recurring Template Pattern' so that 
// the ValueGetter can instantiate the Evaluator itself. It also implements INodeVisitor interface 
// the conventional way - overriding all overloads of Visit virtual method for every type of supported node.
class CodeGenerator : public ValueGetter<CodeGenerator, ASTNode*, Quadruples>, public ASTNodeVisitor
{
private:
    static std::vector<Quadruples> instructions;
    static std::vector<std::pair<std::string, std::string>> inUseLabels;  // is this just the end always? get rid of one string?
    unsigned int labelIndex = 0;

    void ProcessAssignment(const BinaryASTNode& n);
    const std::string ReverseOp(const std::string& op);
public:
    void GenerateTAC(ASTNode* n);
    void GenerateAssembly();

    // Inherited via ASTNodeVisitor
    void Visit(ASTNode& n)               override;
    void Visit(UnaryASTNode& n)          override;
    void Visit(BinaryASTNode& n)         override;
    void Visit(IntegerNode& n)           override;
    void Visit(IdentifierNode& n)        override;
    void Visit(UnaryOperationNode& n)    override;
    void Visit(BinaryOperationNode& n)   override;
    void Visit(ConditionNode& n)         override;
    void Visit(IfNode& n)                override;
    void Visit(IfStatementNode& n)       override;
    void Visit(IterationNode& n)         override;
    void Visit(WhileNode& n)             override;
    void Visit(DoWhileNode& n)           override;
    void Visit(StatementBlockNode& n)    override;
    void Visit(CompoundStatementNode& n) override;
    void Visit(DeclareStatementNode& n)  override;
    void Visit(DeclareAssignNode& n)     override;
    void Visit(AssignStatementNode& n)   override;
    void Visit(ReturnStatementNode& n)   override;
    void Visit(EmptyStatementNode& n)    override;
};