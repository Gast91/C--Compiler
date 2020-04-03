#pragma once

// Forward Declarations
class ASTNode;
class UnaryASTNode;
class BinaryASTNode;
class IntegerNode;
class IdentifierNode;
class BinaryOperationNode;
class ConditionNode;
class IfNode;
class WhileNode;
class CompoundStatementNode;
class DeclareStatementNode;
class AssignStatementNode;
class EmptyStatementNode;

// Pure Abstract Class/Interface that all classes that "walk" the AST should inherit from
// Based on the Visitor Pattern
// Each member of the ASTNode family defines an accept function that accepts the visitor class
// and calls its visit methods based on the ASTNode class that called it.
class ASTNodeVisitor
{
public:
	virtual ~ASTNodeVisitor() = default;
	virtual void Visit(ASTNode& n) = 0;
	virtual void Visit(UnaryASTNode& n) = 0;
	virtual void Visit(BinaryASTNode& n) = 0;
	virtual void Visit(IntegerNode& n) = 0;
	virtual void Visit(IdentifierNode& n) = 0;
	virtual void Visit(BinaryOperationNode& n) = 0;
	virtual void Visit(ConditionNode& n) = 0;
	virtual void Visit(IfNode& n) = 0;
	virtual void Visit(WhileNode& n) = 0;
	virtual void Visit(CompoundStatementNode& n) = 0;
	virtual void Visit(DeclareStatementNode& n) = 0;
	virtual void Visit(AssignStatementNode& n) = 0;
	virtual void Visit(EmptyStatementNode& n) = 0;
};

// Inheriting Classes:
// ASTVisualizer
// SemanticAnalyser
// Code Generator
// Interpreter (NOT BEING IMPLEMENTED)