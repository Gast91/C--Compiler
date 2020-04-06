#pragma once
#include <vector>
#include <fstream>

#include "Visitor.h"
#include "Utility.h"

class ASTVisualizer : public ASTNodeVisitor
{
private:
    std::vector<std::string> config;
    std::ofstream out;
    bool consoleOutput;
public:
    ASTVisualizer(bool console = true) noexcept;
    ~ASTVisualizer() = default;

    void PrintAST(ASTNode& n);

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