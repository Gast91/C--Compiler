#pragma once
#include <vector>
#include <sstream>
#include <fstream>

#include "Visitor.h"

class ASTPrinterJson : public ASTNodeVisitor
{
private:
    std::vector<std::string> config;
    std::ofstream out;

    std::string GenerateID(const ASTNode* node, const char* ID);
    std::string GenerateJSONHeader(std::ofstream& out, const ASTNode* root, const char* rootID, std::vector<std::string>& config);
    void GenerateJSONFooter(std::ofstream& out, const std::vector<std::string>& config);
    std::string GenerateJSON(std::ofstream& out, const ASTNode* node, const char* ID, std::string& parentID, std::string name, std::vector<std::string>& config);
public:
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