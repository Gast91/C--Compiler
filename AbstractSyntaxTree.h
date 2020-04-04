#pragma once
#include <iostream>

#include "Utility.h"
#include "Visitor.h"
#include "Token.h"

// Base Node class
class ASTNode
{
public:
	std::string parentID;
public:
	ASTNode() {}
	virtual ~ASTNode() {}

	// Function allowing the implementation of the visitor pattern
	virtual void Accept(ASTNodeVisitor& v) = 0;
	virtual void SetChildrenPrintID(const std::string pID) = 0;
};

// Abstract Syntax Tree Node with one branch or leaf
class UnaryASTNode : public ASTNode
{
public:
	ASTNode* expr;
public:
	UnaryASTNode(ASTNode* n) : ASTNode(), expr(n) {}
	virtual ~UnaryASTNode() { delete expr; }

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { expr->parentID = pID; }
};

class UnaryOperationNode : public UnaryASTNode
{
public:
	TokenPair op;
public:
	UnaryOperationNode(TokenPair t, ASTNode* n) : UnaryASTNode(n), op(t) {}
	virtual ~UnaryOperationNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { expr->parentID = pID; }
};

// Abstract Syntax Tree Node with two branches or leaves
class BinaryASTNode : public ASTNode
{
public:
	ASTNode*   left;
	ASTNode*   right;
	TokenPair  op;
public:
	BinaryASTNode(ASTNode* l, TokenPair o, ASTNode* r) : ASTNode(), left(l), op(o), right(r) {}
	virtual ~BinaryASTNode()
	{
		delete left;
		delete right;
	}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { left->parentID = pID; right->parentID = pID; }
};

// Node representing a number(integer) literal
class IntegerNode : public ASTNode       // this doesnt have a token anymore
{
public:
	int value;
public:
	IntegerNode(const std::string& val) : ASTNode(), value(std::stoi(val)) {}
	virtual ~IntegerNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { /* No children */ }
};

// Node representing an identifier
class IdentifierNode : public ASTNode    // this doesnt have a token anymore - NOT USED ATM ALSO
{
public:
	Token type;
	std::string value;
public:
	IdentifierNode(const std::string& val, const Token t = Token::UNKNOWN) : ASTNode(), type(t), value(val) {}
	virtual ~IdentifierNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { /* No children */ }
};

// Node representing a binary operation (Addition, Subtraction, Multiplication or Division)
class BinaryOperationNode : public BinaryASTNode   // very similar print with condition
{
public:
	BinaryOperationNode(ASTNode* l, TokenPair o, ASTNode* r) : BinaryASTNode(l, o, r) {}
	virtual ~BinaryOperationNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
};

class ConditionNode : public BinaryASTNode   // very similar print with binaryOperator
{
public:
	ConditionNode(ASTNode* l, TokenPair& o, ASTNode* r) : BinaryASTNode(l, o, r) {}
	virtual ~ConditionNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
};

// this should inherit from a ternary ASTNode
class IfNode : public ASTNode
{
public:
	ASTNode* condition;
	ASTNode* body;
public:
	IfNode(ASTNode* cond, ASTNode* b) : ASTNode(), condition(cond), body(b) {}
	virtual ~IfNode()
	{
		delete condition;
		if (body) delete body;
	}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { condition->parentID = pID; body->parentID = pID; }
};

class WhileNode : public ASTNode  // copy of if right now - if will change later
{
public:
	ASTNode* condition;
	ASTNode* body;
public:
	WhileNode(ASTNode* cond, ASTNode* b) : ASTNode(), condition(cond), body(b) {}
	virtual ~WhileNode()
	{
		delete condition;
		if (body) delete body;
	}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { condition->parentID = pID; body->parentID = pID; }
};

class CompoundStatementNode : public ASTNode  // subclass same with some other?
{
public:
	std::vector<ASTNode*> statements;
public:
	CompoundStatementNode() : ASTNode() {}
	virtual ~CompoundStatementNode() { for (const auto& statement : statements) delete statement; }

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { for (const auto& statement : statements) statement->parentID = pID; }

	void Push(ASTNode* statement) { statements.push_back(statement); }
};

class DeclareStatementNode : public ASTNode
{
public:
	IdentifierNode* identifier;
	TokenPair type;
public:
	DeclareStatementNode(IdentifierNode* ident, TokenPair t) : ASTNode(), identifier(ident), type(t) { identifier->type = t.second; }
	virtual ~DeclareStatementNode() { delete identifier; }

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) {identifier->parentID = pID; }
};

class AssignStatementNode : public BinaryASTNode
{
public:
	AssignStatementNode(IdentifierNode* ident, ASTNode* expr) : BinaryASTNode(ident, { "=", Token::ASSIGN }, expr) {}
	virtual ~AssignStatementNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
};

class ReturnStatementNode : public UnaryASTNode
{
public:
	ReturnStatementNode(ASTNode* n) : UnaryASTNode(n) {}
	virtual ~ReturnStatementNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { expr->parentID = pID; }
};

class EmptyStatementNode : public ASTNode
{
public:
	EmptyStatementNode() : ASTNode() {}
	virtual ~EmptyStatementNode() {}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string pID) { /* No Children */ }
};

/*
Fix/Add:  -Fix all children pointers to reflect what the child is 
		   rather than plain ASTNode? might not be needed anymore
		  -IFNode should be ternary - condition, else, vector of elseIf's (can be IfNodes)
		  -Store type in identifiers, literals?
*/