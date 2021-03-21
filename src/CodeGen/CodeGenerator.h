#pragma once
#include <optional>

#include "../AST/Visitor.h"
#include "../Util/ModuleManager.h"

#define OPTIMIZE_TEMPS
// Optimization flag enables the recycling of already processed temporary variables
// and prevents -> _t0 = a * b -> c = _t0 and instead optimizes to c = a * b.
#ifdef OPTIMIZE_TEMPS
#define fetch_instr(x) Temporary::CheckAndRecycle(GetValue(x))
#else
#define fetch_instr(x) GetValue(x)
#endif // OPTIMIZE_TEMPS

// Type of intermediate representation commands/instructions - Used in assembly generation
enum class CmdType { ARITHM, RELAT, LOG, UNARY, IF, LABEL, GOTO, COPY, RET, REG, NONE };

struct Command
{
    std::string value;
    CmdType type;
};

struct Operand
{
    CmdType type; // this is encoded twice for command and operand! remove from here?
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

// Handles the recycling (if enabled) and creation of new temporary variables
// that are used in both intermediate code generation and assembly generation
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

// Handles the creation of new labels as well as the jmp label
// distribution to conditions that request a jump location
class Label
{
private:
    static int labelCount;
    static std::vector<std::string> cmpJmpLabels;
    static int nextCmpLabel;
public:
    static const Operand NewLabel() { return Operand{ CmdType::LABEL, "_L" + std::to_string(labelCount), "_L" + std::to_string(labelCount++) }; }
    static void AddCmpLabel(const std::string& nextLabel) { cmpJmpLabels.push_back(nextLabel); }
    static const std::string GetCmpLabel() { return cmpJmpLabels.at(nextCmpLabel++); }
};


// CodeGenerator derives from ValueGetter by the 'Curiously Recurring Template Pattern' so that 
// the ValueGetter can instantiate the Evaluator itself. It also implements INodeVisitor interface 
// the conventional way - overriding all overloads of Visit virtual method for every type of supported node.
class CodeGenerator : public ValueGetter<CodeGenerator, ASTNode*, Quadruples>, public ASTNodeVisitor,
                      public IObserver<>, public IObserver<ASTNode>, public IObserver<bool>
{
private:
    static std::vector<Quadruples> instructions;

    std::stringstream tac, x86;

    ASTNode* root = nullptr;

    bool shouldRun  = false;
    bool semSuccess = false;

    void ProcessAssignment(const BinaryASTNode& n);
    void ProcessBinOp(const BinaryASTNode& n, CmdType type);
    const std::string ReverseOp(const std::string& op) const;

    void GenerateTAC();
    void GenerateAssembly();
public:
    std::string GetTAC() const { return tac.str(); }
    std::string Getx86() const { return x86.str(); }   // TEMPS - NO COPY

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

    // Inherited via IObserver - Observing Module Manager
    virtual bool ShouldRun()  const override  { return shouldRun; }
    virtual void SetToRun()         override  { shouldRun = true; }
    virtual void Update()           override;
    virtual void Reset()            override;

    // Inherited via IObserver - Observing AST Changes
    virtual void Update(ASTNode* n) override  { root = n; }

    // Inherited via IObserver - Observing Semantic Analyzer State
    virtual void Update(bool* run)  override  { semSuccess = !*run; }  // ???
};