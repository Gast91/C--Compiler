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
	ASTVisualizer(bool console = true);
	~ASTVisualizer();

	void PrintAST(ASTNode& n);

	// Inherited via ASTNodeVisitor
	virtual void Visit(ASTNode& n)               override;
	virtual void Visit(UnaryASTNode& n)          override;
	virtual void Visit(BinaryASTNode& n)         override;
	virtual void Visit(IntegerNode& n)           override;
	virtual void Visit(IdentifierNode& n)        override;
	virtual void Visit(UnaryOperationNode& n)    override;
	virtual void Visit(BinaryOperationNode& n)   override;
	virtual void Visit(ConditionNode& n)         override;
	virtual void Visit(IfNode& n)                override;
	virtual void Visit(WhileNode& n)             override;
	virtual void Visit(CompoundStatementNode& n) override;
	virtual void Visit(DeclareStatementNode& n)  override;
	virtual void Visit(AssignStatementNode& n)   override;
	virtual void Visit(ReturnStatementNode& n)   override;
	virtual void Visit(EmptyStatementNode& n)    override;
};