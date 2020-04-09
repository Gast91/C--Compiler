#pragma once
#include <iostream>
#include <memory>

#include "Utility.h"
#include "Visitor.h"
#include "Token.h"

using ASTNodePtr               = std::unique_ptr<ASTNode>;
using IfNodePtr                = std::unique_ptr<IfNode>;
using IdentifierNodePtr        = std::unique_ptr<IdentifierNode>;
using DeclareStatementNodePtr  = std::unique_ptr<DeclareStatementNode>;
using IfStatementNodePtr       = std::unique_ptr<IfStatementNode>;
using StatementBlockNodePtr    = std::unique_ptr<StatementBlockNode>;
using CompoundStatementNodePtr = std::unique_ptr<CompoundStatementNode>;
using DeclareAssignNodePtr     = std::unique_ptr<DeclareAssignNode>;

// Base Node class
class ASTNode
{
public:
    std::string parentID;
public:
    virtual ~ASTNode() = default;

    // Function allowing the implementation of the visitor pattern
    virtual void Accept(ASTNodeVisitor& v) = 0;
    virtual void SetChildrenPrintID(const std::string& pID) = 0;
};

// Abstract Syntax Tree Node with one branch or leaf
class UnaryASTNode : public ASTNode
{
public:
    ASTNodePtr expr;
public:
    UnaryASTNode(ASTNodePtr n) noexcept : expr(std::move(n)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

class UnaryOperationNode : public UnaryASTNode
{
public:
    TokenPair op;
public:
    UnaryOperationNode(TokenPair t, ASTNodePtr n) noexcept : UnaryASTNode(std::move(n)), op(t) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

// Abstract Syntax Tree Node with two branches or leaves
class BinaryASTNode : public ASTNode
{
public:
    ASTNodePtr left;
    ASTNodePtr right;
    TokenPair  op;
public:
    BinaryASTNode(ASTNodePtr l, TokenPair o, ASTNodePtr r) noexcept : left(std::move(l)), op(o), right(std::move(r)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { left->parentID = pID; right->parentID = pID; }
};

// Node representing a number(integer) literal
class IntegerNode : public ASTNode       // this doesnt have a token anymore - does it need one for the future?
{
public:
    int value;  // should this just be a string for ease?
public:
    IntegerNode(const std::string& val) : value(std::stoi(val)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No children */ }
};

// Node representing an identifier
class IdentifierNode : public ASTNode
{
public:
    Token type;
    std::string offset;
    const std::string name;
    const std::string lineNo;
public:
    IdentifierNode(const std::string& n, const std::string& line, const Token t = Token::UNKNOWN) : name(n), lineNo(line), type(t) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No children */ }
};

// Node representing a binary operation
class BinaryOperationNode : public BinaryASTNode
{
public:
    BinaryOperationNode(ASTNodePtr l, TokenPair o, ASTNodePtr r) noexcept : BinaryASTNode(std::move(l), o, std::move(r)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class ConditionNode : public BinaryASTNode
{
public:
    ConditionNode(ASTNodePtr l, TokenPair& o, ASTNodePtr r) noexcept : BinaryASTNode(std::move(l), o, std::move(r)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

// Node representing an IF condition body or an ELSE_IF condition body. Part of an umbrella IfStatementNode
class IfNode : public ASTNode
{
public:
    ASTNodePtr condition;
    ASTNodePtr body;
    std::string type;  // IF or ELSE_IF used only for visualization
public:
    IfNode(ASTNodePtr b, ASTNodePtr cond) noexcept : body(std::move(b)),  condition(std::move(cond)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { condition->parentID = pID; body->parentID = pID; }
};

// Node representing a collection of an IFNode, several ELSE_IF and an ELSE CompoundStatementNode
class IfStatementNode : public ASTNode
{
public:
    std::vector<IfNodePtr> ifNodes;
    ASTNodePtr elseBody;
public:

    void AddNode(IfNodePtr node)
    {
        ifNodes.empty() ? node->type = "IF" : node->type = "ELSEIF";
        ifNodes.push_back(std::move(node));
    }
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override
    {
        for (const auto& ifN : ifNodes) ifN->parentID = pID;
        elseBody->parentID = pID;
    }
};

class IterationNode : public ASTNode
{
public:
    ASTNodePtr condition;
    ASTNodePtr body;
public:
    IterationNode(ASTNodePtr cond, ASTNodePtr b) noexcept : condition(std::move(cond)), body(std::move(b)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { condition->parentID = pID; body->parentID = pID; }
};

class WhileNode : public IterationNode
{
public:
    WhileNode(ASTNodePtr cond, ASTNodePtr b) noexcept : IterationNode(std::move(cond), std::move(b)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class DoWhileNode : public IterationNode
{
public:
    DoWhileNode(ASTNodePtr cond, ASTNodePtr b) noexcept : IterationNode(std::move(cond), std::move(b)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class CompoundStatementNode : public ASTNode
{
public:
    std::vector<ASTNodePtr> statements;
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { for (const auto& statement : statements) statement->parentID = pID; }

    void Push(ASTNodePtr statement) { statements.push_back(std::move(statement)); }
};

class StatementBlockNode : public CompoundStatementNode
{
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class DeclareStatementNode : public ASTNode
{
public:
    IdentifierNodePtr identifier;
    TokenPair type;
public:
    DeclareStatementNode(IdentifierNodePtr ident, TokenPair t) noexcept : identifier(std::move(ident)), type(t) { identifier->type = t.second; }

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override {identifier->parentID = pID; }
};

class DeclareAssignNode : public BinaryASTNode
{
public:
    DeclareAssignNode(DeclareStatementNodePtr decl, ASTNodePtr expr) noexcept : BinaryASTNode(std::move(decl), { "=", Token::ASSIGN }, std::move(expr)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class AssignStatementNode : public BinaryASTNode
{
public:
    AssignStatementNode(IdentifierNodePtr ident, ASTNodePtr expr) noexcept : BinaryASTNode(std::move(ident), { "=", Token::ASSIGN }, std::move(expr)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
};

class ReturnStatementNode : public UnaryASTNode
{
public:
    ReturnStatementNode(ASTNodePtr n) noexcept : UnaryASTNode(std::move(n)) {}

    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) override { expr->parentID = pID; }
};

class EmptyStatementNode : public ASTNode
{
public:
    void Accept(ASTNodeVisitor& v) override { v.Visit(*this); }
    void SetChildrenPrintID(const std::string& pID) noexcept override { /* No Children */ }
};

/*
Fix/Add:    -Store type in literals?
*/