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
	ASTNode() = default;
	virtual ~ASTNode() = default;

	// Function allowing the implementation of the visitor pattern
	virtual void Accept(ASTNodeVisitor& v) = 0;
	virtual void SetChildrenPrintID(const std::string& pID) = 0;
};

// Abstract Syntax Tree Node with one branch or leaf
class UnaryASTNode : public ASTNode
{
public:
	ASTNode* expr;
public:
	UnaryASTNode(ASTNode* n) : expr(n) {}
	virtual ~UnaryASTNode() { delete expr; }

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { expr->parentID = pID; }
};

class UnaryOperationNode : public UnaryASTNode
{
public:
	TokenPair op;
public:
	UnaryOperationNode(TokenPair t, ASTNode* n) : UnaryASTNode(n), op(t) {}
	virtual ~UnaryOperationNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { expr->parentID = pID; }
};

// Abstract Syntax Tree Node with two branches or leaves
class BinaryASTNode : public ASTNode
{
public:
	ASTNode*   left;
	ASTNode*   right;
	TokenPair  op;
public:
	BinaryASTNode(ASTNode* l, TokenPair o, ASTNode* r) : left(l), op(o), right(r) {}
	virtual ~BinaryASTNode()
	{
		delete left;
		delete right;
	}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { left->parentID = pID; right->parentID = pID; }
};

// Node representing a number(integer) literal
class IntegerNode : public ASTNode       // this doesnt have a token anymore - does it need one for the future?
{
public:
	int value;
public:
	IntegerNode(const std::string& val) : value(std::stoi(val)) {}
	virtual ~IntegerNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { /* No children */ }
};

// Node representing an identifier
class IdentifierNode : public ASTNode
{
public:
	Token type;
	std::string name;
public:
	IdentifierNode(const std::string& n, const Token t = Token::UNKNOWN) : type(t), name(n) {}
	virtual ~IdentifierNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { /* No children */ }
};

// Node representing a binary operation (Addition, Subtraction, Multiplication or Division)
class BinaryOperationNode : public BinaryASTNode
{
public:
	BinaryOperationNode(ASTNode* l, TokenPair o, ASTNode* r) : BinaryASTNode(l, o, r) {}
	virtual ~BinaryOperationNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
};

class ConditionNode : public BinaryASTNode
{
public:
	ConditionNode(ASTNode* l, TokenPair& o, ASTNode* r) : BinaryASTNode(l, o, r) {}
	virtual ~ConditionNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
};

// this should inherit from a ternary ASTNode
class IfNode : public ASTNode
{
public:
	ASTNode* condition;
	ASTNode* body;
public:
	IfNode(ASTNode* cond, ASTNode* b) : condition(cond), body(b) {}
	virtual ~IfNode()
	{
		delete condition;
		if (body) delete body;
	}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { condition->parentID = pID; body->parentID = pID; }
};

class WhileNode : public ASTNode  // copy of if right now - if will change later
{
public:
	ASTNode* condition;
	ASTNode* body;
public:
	WhileNode(ASTNode* cond, ASTNode* b) : condition(cond), body(b) {}
	virtual ~WhileNode()
	{
		delete condition;
		if (body) delete body;
	}

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { condition->parentID = pID; body->parentID = pID; }
};

class CompoundStatementNode : public ASTNode
{
public:
	std::vector<ASTNode*> statements;
public:
	CompoundStatementNode() = default;
	virtual ~CompoundStatementNode() { for (const auto& statement : statements) delete statement; }

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { for (const auto& statement : statements) statement->parentID = pID; }

	void Push(ASTNode* statement) { statements.push_back(statement); }
};

class DeclareStatementNode : public ASTNode
{
public:
	IdentifierNode* identifier;
	TokenPair type;
public:
	DeclareStatementNode(IdentifierNode* ident, TokenPair t) : identifier(ident), type(t) { identifier->type = t.second; }
	virtual ~DeclareStatementNode() { delete identifier; }

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) {identifier->parentID = pID; }
};

class AssignStatementNode : public BinaryASTNode
{
public:
	AssignStatementNode(IdentifierNode* ident, ASTNode* expr) : BinaryASTNode(ident, { "=", Token::ASSIGN }, expr) {}
	virtual ~AssignStatementNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
};

class ReturnStatementNode : public UnaryASTNode
{
public:
	ReturnStatementNode(ASTNode* n) : UnaryASTNode(n) {}
	virtual ~ReturnStatementNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { expr->parentID = pID; }
};

class EmptyStatementNode : public ASTNode
{
public:
	EmptyStatementNode() = default;
	virtual ~EmptyStatementNode() = default;

	virtual void Accept(ASTNodeVisitor& v) { v.Visit(*this); }
	virtual void SetChildrenPrintID(const std::string& pID) { /* No Children */ }
};

/*
Fix/Add:  -Only if/while body can be a compoundNode rather than ASTNode but it doesnt help anyway 
		  -IFNode should be ternary - condition, else, vector of elseIf's (can be IfNodes)
		  -Store type in literals?
*/