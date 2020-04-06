#pragma once
#include <string>
#include <optional>
#include "Token.h"
#include "Visitor.h"

struct Quadruples
{
	std::optional<std::string> op;    // TOKEN_PAIR WOULD BE AMAZING HERE
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
	static int temporaries;
	static int labels;
public:
	void GenerateAssembly(ASTNode* n);

	// Inherited via ASTNodeVisitor
	virtual void Visit(ASTNode& n) override;
	virtual void Visit(UnaryASTNode& n) override;
	virtual void Visit(BinaryASTNode& n) override;
	virtual void Visit(IntegerNode& n) override;
	virtual void Visit(IdentifierNode& n) override;
	virtual void Visit(UnaryOperationNode& n) override;
	virtual void Visit(BinaryOperationNode& n) override;
	virtual void Visit(ConditionNode& n) override;
	virtual void Visit(IfNode& n) override;
	virtual void Visit(WhileNode& n) override;
	virtual void Visit(CompoundStatementNode& n) override;
	virtual void Visit(DeclareStatementNode& n) override;
	virtual void Visit(AssignStatementNode& n) override;
	virtual void Visit(ReturnStatementNode& n) override;
	virtual void Visit(EmptyStatementNode& n) override;
};