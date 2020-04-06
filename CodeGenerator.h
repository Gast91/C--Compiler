#pragma once
#include <string>
#include <optional>

#include "Token.h"
#include "Visitor.h"

class Temporary
{
private:
    static int tempCount;
public:
    static const std::string NewTemporary() { return "_t" + std::to_string(tempCount++);  }
};

class Label
{
private:
    static int labelCount;
public:
    static const std::string NewLabel() { return "_L" + std::to_string(labelCount++); }
};

struct Quadruples
{
    std::optional<std::string> op;
    std::optional<std::string> src1;
    std::optional<std::string> src2;
    std::optional<std::string> dest;
};

using ThreeAddressCode = std::vector<Quadruples>;

// CodeGenerator derives from ValueGetter by the 'Curiously Recurring Template Pattern' so that 
// the ValueGetter can instantiate the Evaluator itself. It also implements INodeVisitor interface 
// the conventional way - overriding all overloads of Visit virtual method for every type of supported node.
class CodeGenerator : public ValueGetter<CodeGenerator, ASTNode*, Quadruples>, public ASTNodeVisitor
{
private:
    static ThreeAddressCode instructions;
public:
    void GenerateAssembly(ASTNode* n);

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
    void Visit(WhileNode& n)             override;
    void Visit(CompoundStatementNode& n) override;
    void Visit(DeclareStatementNode& n)  override;
    void Visit(AssignStatementNode& n)   override;
    void Visit(ReturnStatementNode& n)   override;
    void Visit(EmptyStatementNode& n)    override;
};